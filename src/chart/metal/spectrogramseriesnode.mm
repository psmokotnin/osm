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
#include "spectrogramseriesnode.h"
#include "../spectrogramplot.h"
#include <Metal/Metal.h>

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

SpectrogramSeriesNode::SpectrogramSeriesNode(QQuickItem *item) : XYSeriesNode(item),
    m_history(), m_refreshBuffers(true),
    m_min(0), m_mid(0), m_max(0), m_pointsPerOctave(0), m_sourceSize(0),
    m_timer(),
    m_pipeline(nullptr), m_indiciesBuffer(nullptr)
{
    connect(source().get(), &Abstract::Source::readyRead, this, &SpectrogramSeriesNode::updateHistory);
}

SpectrogramSeriesNode::~SpectrogramSeriesNode()
{
    [id_cast(MTLRenderPipelineState, m_pipeline) release];
}

void SpectrogramSeriesNode::initRender()
{
    auto library = id_cast(MTLLibrary, m_library);
    auto vertexProgram = [library newFunctionWithName:@"logColorVertex" ];
    auto fragmentProgram = [library newFunctionWithName:@"fillPointColor" ];

    MTLVertexDescriptor *inputLayout = [MTLVertexDescriptor vertexDescriptor];
    inputLayout.attributes[0].format = MTLVertexFormatFloat2;
    inputLayout.attributes[0].offset = 0;
    inputLayout.attributes[0].bufferIndex = 0;

    inputLayout.attributes[1].format = MTLVertexFormatFloat4;
    inputLayout.attributes[1].offset = 2 * sizeof(float);
    inputLayout.attributes[1].bufferIndex = 0;
    inputLayout.layouts[0].stride = 6 * sizeof(float);

    auto pipelineStateDescriptor = static_cast<MTLRenderPipelineDescriptor *>(allocPipelineStateDescriptor());
    pipelineStateDescriptor.vertexDescriptor = inputLayout;
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;

    NSError *error = nil;
    m_pipeline = [
                     id_cast(MTLDevice, m_device)
                     newRenderPipelineStateWithDescriptor: pipelineStateDescriptor
                     error: &error];

    if (!m_pipeline) {
        const QString msg = QString::fromNSString(error.localizedDescription);
        qDebug() << "Failed to create render pipeline state: %s", qPrintable(msg);
        plot()->setRendererError("Failed to create render pipeline state " + msg);
        m_pipeline = nullptr;
        return;
    }

    [pipelineStateDescriptor release];

    m_matrixBuffer = [
                         id_cast(MTLDevice, m_device)
                         newBufferWithLength: 16 * sizeof(float)
                         options: MTLResourceStorageModeShared
                     ];
}

void SpectrogramSeriesNode::synchronizeSeries()
{
    if (!m_source) {
        return;
    }
    synchronizeMatrix();
    if (auto *spectrogramPlot = dynamic_cast<SpectrogramPlot *>(plot())) {
        if (
            m_pointsPerOctave != spectrogramPlot->pointsPerOctave() ||
            m_sourceSize != m_source->size()
        ) {
            m_refreshBuffers = true;
            m_history.clear();
        }
        m_sourceSize = m_source->size();
        m_pointsPerOctave = spectrogramPlot->pointsPerOctave();
        m_min = spectrogramPlot->min();
        m_mid = spectrogramPlot->mid();
        m_max = spectrogramPlot->max();
        m_plotActive = spectrogramPlot->active();
    }
}

void SpectrogramSeriesNode::updateHistory()
{
    //QSGRenderThread
    Q_ASSERT(thread() == QThread::currentThread());

    float floor = -140.f;
    float alpha;

    std::lock_guard guard(m_active);
    historyRowData rowData;
    historyRow row;
    row.time = static_cast<int>(m_timer.restart());
    row.data.reserve(m_pointsPerOctave * 11);
    float value = 0.f;
    static const QColor qred("#F44336"), qgreen("#8BC34A"), qblue("#2196F3");
    QColor pointColor;

    auto mix = [] (const QColor & first, const QColor & second, qreal k) {
        QColor mixedColor;
        mixedColor.setRedF(  k * (second.redF()   - first.redF())   + first.redF());
        mixedColor.setBlueF( k * (second.blueF()  - first.blueF())  + first.blueF());
        mixedColor.setGreenF(k * (second.greenF() - first.greenF()) + first.greenF());
        return mixedColor;
    };
    auto accumalte = [this, &value] (const unsigned int &i) {
        if (i == 0) {
            return ;
        }
        value += m_source->module(i) * m_source->module(i);
    };
    auto collected = [&] (const float & start, const float & end, const unsigned int &) {

        value = 10 * log10f(value) + LEVEL_NORMALIZATION;

        alpha = 1.0f;
        if (!std::isnormal(value) || value < floor) {
            value = floor;
        }
        if (value > m_max) {
            value = m_max;
        }

        if (value < m_min) {
            // transparent -> blue
            pointColor = qblue;
            pointColor = qblue;
            alpha = (value - floor) / (m_min   - floor);
        } else if (value < m_mid) {
            // blue -> green
            pointColor = mix(qblue, qgreen, static_cast<qreal>((value - m_min) / (m_mid - m_min)));
        } else  {
            // green -> red
            pointColor = mix(qgreen, qred, static_cast<qreal>((value - m_mid) / (m_max - m_mid)));
        }

        historyPoint rgb;
        rgb[0] = (start + end) / 2.f;
        rgb[1] = static_cast<float>(pointColor.redF());
        rgb[2] = static_cast<float>(pointColor.greenF());
        rgb[3] = static_cast<float>(pointColor.blueF());
        rgb[4] = alpha;
        row.data.push_back(rgb);

        value = 0;
    };

    if (m_plotActive) {
        iterate(m_pointsPerOctave, accumalte, collected);

        m_history.push_back(std::move(row));
        if (m_history.size() > MAX_HISTORY) {
            m_history.pop_front();
        }
    }
}

void SpectrogramSeriesNode::renderSeries()
{
    if (!m_pipeline) {
        return;
    }
    if (!m_source->size() || m_history.empty() || !m_pointsPerOctave) {
        clearRender();
        return;
    }

    unsigned int maxBufferSize = MAX_HISTORY * (m_pointsPerOctave * 12 + 4) * 6,
                 verticiesCount = 0, maxIndicesCount = 0, indicesCount = 0;

    if (m_refreshBuffers) {
        [id_cast(MTLBuffer, m_vertexBuffer) release];
        [id_cast(MTLBuffer, m_indiciesBuffer) release];
        m_vertexBuffer = m_indiciesBuffer = nullptr;
    }
    float *vertex_ptr = vertexBuffer(maxBufferSize);

    auto rowSize = m_history[0].data.size();
    maxIndicesCount = MAX_HISTORY * (2 * rowSize + 2);
    if ([id_cast(MTLBuffer, m_indiciesBuffer) length] != maxIndicesCount * sizeof(unsigned int)) {
        [id_cast(MTLBuffer, m_indiciesBuffer) release];
        m_indiciesBuffer = [
                               id_cast(MTLDevice, m_device)
                               newBufferWithLength: maxIndicesCount * sizeof(unsigned int)
                               options: MTLResourceStorageModeShared
                           ];
    }
    auto *indicies_ptr = static_cast<unsigned int *>([id_cast(MTLBuffer, m_indiciesBuffer) contents]);
    if (!indicies_ptr) {
        m_refreshBuffers = true;
        return;
    }

    float t(0), tStep(0);
    unsigned int j = 0, index = 0;
    auto addPoint = [&] (const historyPoint & data, const float & time) {
        if (j + 6  > maxBufferSize) {
            qCritical("out of range");
            return;
        }
        vertex_ptr[j + 0] = data[0];
        vertex_ptr[j + 1] = time;
        vertex_ptr[j + 2] = data[1];
        vertex_ptr[j + 3] = data[2];
        vertex_ptr[j + 4] = data[3];
        vertex_ptr[j + 5] = data[4];
        j += 6;
        verticiesCount ++;
    };

    auto addIndices = [&](const unsigned int value) {
        if (indicesCount >= maxIndicesCount) {
            qCritical("out of indices count");
            return;
        }
        indicies_ptr[indicesCount++] = value;
    };

    for (auto row = m_history.crbegin(); row != m_history.crend(); ++row) {
        const historyRowData *rowData = &(row->data);
        tStep = row->time / 1000.f;
        if (rowData->size() != rowSize) {
            t += tStep;
            continue;
        }
        unsigned int i = 0;
        addIndices(index + i);
        for (; i < rowData->size(); ++i) {
            addPoint(rowData->at(i), t);
            addIndices(index + i);
            addIndices(index + i + rowSize);
        }
        addIndices(index + (i - 1) + rowSize);
        t += tStep;
        index += rowSize;
    }
    indicesCount--;

    void *matrix_ptr = [id_cast(MTLBuffer, m_matrixBuffer) contents];
    if (matrix_ptr) {
        m_matrix.copyDataTo(static_cast<float *>(matrix_ptr));
    }

    auto encoder = id_cast(MTLRenderCommandEncoder, commandEncoder());
    [encoder setVertexBuffer: id_cast(MTLBuffer, m_vertexBuffer) offset: 0 atIndex: 0];
    [encoder setVertexBuffer: id_cast(MTLBuffer, m_matrixBuffer) offset: 0 atIndex: 1];
    [encoder setRenderPipelineState: id_cast(MTLRenderPipelineState, m_pipeline)];
    [encoder drawIndexedPrimitives: MTLPrimitiveTypeTriangleStrip
             indexCount: indicesCount
             indexType: MTLIndexTypeUInt32
             indexBuffer: id_cast(MTLBuffer, m_indiciesBuffer)
             indexBufferOffset: 0
            ];

    [encoder endEncoding];
    m_refreshBuffers = false;
}

void SpectrogramSeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMin, m_yMax, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), 0);
}

const Shared::Source &SpectrogramSeriesNode::source() const
{
    return m_source;
}

} // namespace chart
