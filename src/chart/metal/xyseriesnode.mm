/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "xyseriesnode.h"
#include "../xyplot.h"
#include <Metal/Metal.h>

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

XYSeriesNode::XYSeriesNode(QQuickItem *item) : SeriesNode(item),
    m_matrix(),
    m_xMin(0.f), m_xMax(0.f), m_yMin(0.f), m_yMax(0.f),
    m_vertexBuffer(nullptr),  m_matrixBuffer(nullptr)
{
}

XYSeriesNode::~XYSeriesNode()
{
    [id_cast(MTLBuffer, m_vertexBuffer) release];
    [id_cast(MTLBuffer, m_matrixBuffer) release];
}

void XYSeriesNode::synchronizeMatrix()
{
    if (auto *plot = dynamic_cast<XYPlot *>(m_item->parent())) {
        if (
            m_xMin != plot->xAxis()->min() ||
            m_xMax != plot->xAxis()->max() ||
            m_yMin != plot->yAxis()->min() ||
            m_yMax != plot->yAxis()->max()
        ) {

            m_xMin = plot->xAxis()->min();
            m_xMax = plot->xAxis()->max();
            m_yMin = plot->yAxis()->min();
            m_yMax = plot->yAxis()->max();
            updateMatrix();
        }
    }
}

void *XYSeriesNode::initLine(QString vertexProgramName)
{
    auto library = id_cast(MTLLibrary, m_library);
    auto vertexProgram = [library newFunctionWithName:vertexProgramName.toNSString() ];
    auto fragmentProgram = [library newFunctionWithName:@"fillColor" ];

    MTLVertexDescriptor *inputLayout = [MTLVertexDescriptor vertexDescriptor];
    inputLayout.attributes[0].format = MTLVertexFormatFloat2;
    inputLayout.attributes[0].offset = 0;
    inputLayout.attributes[0].bufferIndex = 0;

    inputLayout.attributes[1].format = MTLVertexFormatFloat2;
    inputLayout.attributes[1].offset = 2 * sizeof(float);
    inputLayout.attributes[1].bufferIndex = 0;

    inputLayout.attributes[2].format = MTLVertexFormatFloat;
    inputLayout.attributes[2].offset = 4 * sizeof(float);
    inputLayout.attributes[2].bufferIndex = 0;

    inputLayout.layouts[0].stride = 5 * sizeof(float);

    auto pipelineStateDescriptor = static_cast<MTLRenderPipelineDescriptor *>(allocPipelineStateDescriptor());
    pipelineStateDescriptor.vertexDescriptor = inputLayout;
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;

    NSError *error = nil;
    auto pipeline = [
                        id_cast(MTLDevice, m_device)
                        newRenderPipelineStateWithDescriptor: pipelineStateDescriptor
                        error: &error];

    if (!pipeline) {
        const QString msg = QString::fromNSString(error.localizedDescription);
        qDebug() << "Failed to create render pipeline state: %s", qPrintable(msg);
        plot()->setRendererError("Failed to create render pipeline state " + msg);
        return nullptr;
    }


    [pipelineStateDescriptor release];

    m_matrixBuffer = [
                         id_cast(MTLDevice, m_device)
                         newBufferWithLength: 16 * sizeof(float)
                         options: MTLResourceStorageModeShared
                     ];

    return pipeline;
}

float *XYSeriesNode::vertexBuffer(unsigned int maxBufferSize)
{
    if ([id_cast(MTLBuffer, m_vertexBuffer) length] != maxBufferSize * sizeof(float)) {
        [id_cast(MTLBuffer, m_vertexBuffer) release];
        m_vertexBuffer = [
                             id_cast(MTLDevice, m_device)
                             newBufferWithLength: maxBufferSize * sizeof(float)
                             options: MTLResourceStorageModeShared
                         ];
    }
    return static_cast<float *>([id_cast(MTLBuffer, m_vertexBuffer) contents]);
}

void XYSeriesNode::encodeLine(void *pipeline, unsigned int verticiesCount)
{
    if (!pipeline) {
        return;
    }
    void *matrix_ptr = [id_cast(MTLBuffer, m_matrixBuffer) contents];
    if (matrix_ptr) {
        m_matrix.copyDataTo(static_cast<float *>(matrix_ptr));
    }

    auto encoder = id_cast(MTLRenderCommandEncoder, commandEncoder());
    [encoder setVertexBuffer: id_cast(MTLBuffer, m_vertexBuffer) offset: 0 atIndex: 0];
    [encoder setVertexBuffer: id_cast(MTLBuffer, m_matrixBuffer) offset: 0 atIndex: 1];
    [encoder setVertexBuffer: id_cast(MTLBuffer, m_sizeBuffer)   offset: 0 atIndex: 2];
    [encoder setVertexBuffer: id_cast(MTLBuffer, m_widthBuffer)  offset: 0 atIndex: 3];
    [encoder setFragmentBuffer: id_cast(MTLBuffer, m_colorBuffer) offset: 0 atIndex: 1];
    [encoder setRenderPipelineState: id_cast(MTLRenderPipelineState, pipeline)];
    [encoder drawPrimitives: MTLPrimitiveTypeTriangle
             vertexStart: 0
             vertexCount: verticiesCount];

    [encoder endEncoding];
}

float XYSeriesNode::coherenceSpline(const bool &coherence, const float &threshold, const float data[4],
                                    const float &t) const
{
    if (!coherence) {
        return 1.f;
    }
    auto alpha = std::abs(data[0] + data[1] * t + data[2] * t * t + data[3] * t * t * t);
    if (alpha < threshold) {
        alpha = FLT_MIN;
    } else {
        float k = 1.0 / (1.0 - threshold + FLT_MIN);
        float b = -k * threshold;
        alpha = sqrt(k * alpha + b);
    }
    if (alpha > 1.f) {
        alpha = 1.f;
    }
    return alpha;
}

void XYSeriesNode::addLinePoint(float *vertex_ptr, unsigned int &i, unsigned int &verticiesCount,
                                const float &x, const float &y,
                                const float &dX, const float &dY, const float &d)
{
    vertex_ptr[i + 0] = x;
    vertex_ptr[i + 1] = y;

    vertex_ptr[i + 2] = dX;
    vertex_ptr[i + 3] = dY;

    vertex_ptr[i + 4] = d;
    verticiesCount ++;
    i += 5;
}

void XYSeriesNode::addLineSegment(float *vertex_ptr, unsigned int &i, unsigned int &verticiesCount,
                                  const float &fromX, const float &fromY, const float &toX, const float &toY, const float &fromC, const float &toC)
{
    addLinePoint(vertex_ptr, i, verticiesCount, fromX, fromY, toX, toY, fromC);
    addLinePoint(vertex_ptr, i, verticiesCount, fromX, fromY, toX, toY, fromC);
    addLinePoint(vertex_ptr, i, verticiesCount, toX, toY, fromX, fromY, -toC);

    addLinePoint(vertex_ptr, i, verticiesCount, fromX, fromY, toX, toY, fromC);
    addLinePoint(vertex_ptr, i, verticiesCount, toX, toY, fromX, fromY, -toC);
    addLinePoint(vertex_ptr, i, verticiesCount, toX, toY, fromX, fromY, -toC);
}

} // namespace chart
