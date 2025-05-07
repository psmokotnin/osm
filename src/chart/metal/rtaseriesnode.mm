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
#include <Metal/Metal.h>
#include "rtaseriesnode.h"
#include "../rtaplot.h"
#include "math/equalloudnesscontour.h"

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

RTASeriesNode::RTASeriesNode(QQuickItem *item) : XYSeriesNode(item),
    m_pointsPerOctave(0),
    m_mode(Chart::RTAPlot::Mode::Line),
    m_scale(Chart::RTAPlot::Scale::DBfs),
    m_refreshBuffers(true), m_pipelineLine(nullptr), m_pipelineBars(nullptr),
    m_vertexBuffer(nullptr), m_matrixBuffer(nullptr)
{
}

RTASeriesNode::~RTASeriesNode()
{
    [id_cast(MTLRenderPipelineState, m_pipelineLine) release];
    [id_cast(MTLRenderPipelineState, m_pipelineBars) release];
    [id_cast(MTLBuffer, m_vertexBuffer) release];
    [id_cast(MTLBuffer, m_matrixBuffer) release];
}

void RTASeriesNode::initRender()
{
    auto library = id_cast(MTLLibrary, m_library);
    auto vertexProgram = [library newFunctionWithName:@"logLineVertex" ];
    auto barsVertexProgram = [library newFunctionWithName:@"logVertex" ];
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
    m_pipelineLine = [
                         id_cast(MTLDevice, m_device)
                         newRenderPipelineStateWithDescriptor: pipelineStateDescriptor
                         error: &error];
    if (!m_pipelineLine) {
        const QString msg = QString::fromNSString(error.localizedDescription);
        qDebug() << "Failed to create render pipeline state: %s", qPrintable(msg);
        plot()->setRendererError("Failed to create render pipeline state " + msg);
    }

    MTLVertexDescriptor *barsInputLayout = [MTLVertexDescriptor vertexDescriptor];
    barsInputLayout.attributes[0].format = MTLVertexFormatFloat2;
    barsInputLayout.attributes[0].offset = 0;
    barsInputLayout.attributes[0].bufferIndex = 0;
    barsInputLayout.layouts[0].stride = 2 * sizeof(float);

    pipelineStateDescriptor.vertexFunction = barsVertexProgram;
    pipelineStateDescriptor.vertexDescriptor = barsInputLayout;
    m_pipelineBars = [id_cast(MTLDevice,
                              m_device) newRenderPipelineStateWithDescriptor: pipelineStateDescriptor error: &error];
    if (!m_pipelineBars) {
        const QString msg = QString::fromNSString(error.localizedDescription);
        qDebug() << "Failed to create render pipeline state: %s" << qPrintable(msg);
        plot()->setRendererError("Failed to create render pipeline state " + msg);
        m_pipelineBars = nullptr;
    }

    [pipelineStateDescriptor release];

    m_matrixBuffer = [
                         id_cast(MTLDevice, m_device)
                         newBufferWithLength: 16 * sizeof(float)
                         options: MTLResourceStorageModeShared
                     ];
}

void RTASeriesNode::synchronizeSeries()
{
    if (auto *rtaPlot = dynamic_cast<RTAPlot *>(plot())) {
        if (m_pointsPerOctave != rtaPlot->pointsPerOctave()) {
            m_pointsPerOctave = rtaPlot->pointsPerOctave();
        }
        if (m_mode != rtaPlot->mode()) {
            m_mode = rtaPlot->mode();
        }
        if (m_scale != rtaPlot->scale()) {
            m_scale = rtaPlot->scale();
        }
        m_showPeaks = rtaPlot->showPeaks();
    }
    synchronizeMatrix();
}

void RTASeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMin, m_yMax, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), LEVEL_NORMALIZATION);
}

const Shared::Source &RTASeriesNode::source() const
{
    return m_source;
}

void RTASeriesNode::renderSeries()
{
    if (!m_source->frequencyDomainSize()) {
        clearRender();
        return;
    }

    switch (m_mode) {
    case Chart::RTAPlot::Mode::Line:
        renderLine();
        break;
    case Chart::RTAPlot::Mode::Bars:
        renderBars();
        break;
    case Chart::RTAPlot::Mode::Lines:
        renderLines();
        break;

    default:
        Q_UNREACHABLE();
        return;
    }
    m_refreshBuffers = false;
}

void RTASeriesNode::renderLine()
{
    if (!m_pipelineLine) {
        return;
    }

    unsigned int maxBufferSize = (m_pointsPerOctave > 0 ? 12 * m_pointsPerOctave : m_source->frequencyDomainSize() - 1) * 6 * 5;
    unsigned int vertexCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize, 0);
        m_refreshBuffers = true;
    }

    float offset = 0;
    switch (m_scale) {
    case RTAPlot::Scale::DBfs:
        offset = 0;
        break;
    case RTAPlot::Scale::SPL:
    case RTAPlot::Scale::Phon:
        offset = absolute_scale_offset;
    }

    Math::EqualLoudnessContour elc;

    auto addPoint = [ &, this] (auto & i, auto x, auto y, auto dX, auto dY, auto d) {
        m_vertices[i + 0] = x;
        m_vertices[i + 1] = y;

        m_vertices[i + 2] = dX;
        m_vertices[i + 3] = dY;

        m_vertices[i + 4] = d;
        i += 5;
        ++vertexCount;
    };

    auto addSegment = [&] (auto & i, auto fromX, auto fromY, auto toX, auto toY) {
        addPoint(i, fromX, fromY, toX, toY, 1.f);
        addPoint(i, fromX, fromY, toX, toY, 1.f);
        addPoint(i, toX, toY, fromX, fromY, -1.f);

        addPoint(i, fromX, fromY, toX, toY, 1.f);
        addPoint(i, toX, toY, fromX, fromY, -1.f);
        addPoint(i, toX, toY, fromX, fromY, -1.f);
    };

    if (m_pointsPerOctave > 0) {
        float value = 0, lastValue = 0, lastFrequency = 0;

        auto accumalte = [ &, this] (const unsigned int &i) {
            if (i == 0) {
                return ;
            }
            value += m_source->module(i) * m_source->module(i);
        };
        unsigned int i = 0;
        auto collected = [ &, this] (const float & start, const float & end, const unsigned int &count) {
            if (i + 30 > m_vertices.size()) {
                qCritical("out of range");
                return;
            }

            auto frequency = (start + end) / 2;
            value = 10 * log10f(value) + offset;
            if (m_scale == RTAPlot::Scale::Phon) {
                value = elc.phone(frequency, value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            }

            if (lastFrequency > 0) {
                addSegment(i, lastFrequency, lastValue, frequency, value);
            }
            lastValue = value;
            lastFrequency = frequency;
            value = 0;
        };
        iterate(m_pointsPerOctave, accumalte, collected);
    } else {
        unsigned int j = 0, i;
        for (i = 0; i < m_source->frequencyDomainSize() - 1; ++i) {
            auto value1 = 20 * log10f(m_source->module(i    )) + offset;
            auto value2 = 20 * log10f(m_source->module(i + 1)) + offset;
            if (m_scale == RTAPlot::Scale::Phon) {
                value1 = elc.phone(m_source->frequency(i), value1 + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
                value2 = elc.phone(m_source->frequency(i), value2 + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            }
            addSegment(j,
                       m_source->frequency(i),     value1,
                       m_source->frequency(i + 1), value2
                      );
        }
    }

    if (m_refreshBuffers) {
        [id_cast(MTLBuffer, m_vertexBuffer) release];
        m_vertexBuffer = [
                             id_cast(MTLDevice, m_device)
                             newBufferWithLength: m_vertices.size() * sizeof(float)
                             options: MTLResourceStorageModeShared
                         ];
    }
    void *vertex_ptr = [id_cast(MTLBuffer, m_vertexBuffer) contents];
    if (vertex_ptr) {
        memcpy(vertex_ptr, m_vertices.data(), m_vertices.size() * sizeof(float));
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
    [encoder setRenderPipelineState: id_cast(MTLRenderPipelineState, m_pipelineLine)];
    [encoder drawPrimitives: MTLPrimitiveTypeTriangle
             vertexStart: 0
             vertexCount: vertexCount];
    [encoder endEncoding];
}

void RTASeriesNode::renderBars()
{
    if (!m_pipelineBars) {
        return;
    }
    unsigned int maxBufferSize = (m_pointsPerOctave ? m_pointsPerOctave * 12 : m_source->frequencyDomainSize()) * 24;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float offset = 0;
    switch (m_scale) {
    case RTAPlot::Scale::DBfs:
        break;
    case RTAPlot::Scale::SPL:
    case RTAPlot::Scale::Phon:
        offset += absolute_scale_offset;
    }

    unsigned int verticesCollected = 0;
    Math::EqualLoudnessContour elc;

    float value = 0, peak = 0;
    float y = 1 / (m_matrix(0, 0) * width());
    auto accumalte = [ &, this] (const unsigned int &i) {
        if (i == 0) {
            return ;
        }
        value += m_source->module(i) * m_source->module(i);
        peak = std::max(peak, m_source->peakSquared(i));
    };

    unsigned int i = 0;
    auto maxI = maxBufferSize - (m_showPeaks ? 24 : 12);
    auto collected = [ &, this] (const float & start, const float & end, const unsigned int &) {
        if (i > maxI) {
            qCritical("out of range");
            return;
        }

        value = 10 * log10f(value) + offset;
        peak  = 10 * log10f(peak) + offset;
        auto frequencyPoint = (start + end) / 2;
        if (m_scale == RTAPlot::Scale::Phon) {
            value = elc.phone(frequencyPoint, value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            peak  = elc.phone(frequencyPoint, peak + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
        }

        float shiftedStart = std::pow(M_E, std::log(start) + y);
        float shiftedEnd   = std::pow(M_E, std::log(end  ) - y);
        if (shiftedStart > shiftedEnd) {
            std::swap(shiftedStart, shiftedEnd);
        }
        if (shiftedEnd - shiftedStart < 1) {
            shiftedStart = start;
            shiftedEnd = end;
        }

        m_vertices[i + 0] = shiftedStart;
        m_vertices[i + 1] = value;
        m_vertices[i + 2] = shiftedStart;
        m_vertices[i + 3] = m_yMin;
        m_vertices[i + 4] = shiftedEnd;
        m_vertices[i + 5] = value;

        m_vertices[i + 6] = shiftedEnd;
        m_vertices[i + 7] = value;
        m_vertices[i + 8] = shiftedEnd;
        m_vertices[i + 9] = m_yMin;
        m_vertices[i + 10] = shiftedStart;
        m_vertices[i + 11] = m_yMin;

        if (m_showPeaks) {
            m_vertices[i + 12] = shiftedStart;
            m_vertices[i + 13] = peak + 1;
            m_vertices[i + 14] = shiftedStart;
            m_vertices[i + 15] = peak;
            m_vertices[i + 16] = shiftedEnd;
            m_vertices[i + 17] = peak + 1;

            m_vertices[i + 18] = shiftedEnd;
            m_vertices[i + 19] = peak + 1;
            m_vertices[i + 20] = shiftedEnd;
            m_vertices[i + 21] = peak;
            m_vertices[i + 22] = shiftedStart;
            m_vertices[i + 23] = peak;

            i += 24;
            verticesCollected += 12;
        } else {
            i += 12;
            verticesCollected += 6;
        }
        value = 0;
        peak = 0;
    };

    iterate(m_pointsPerOctave, accumalte, collected);

    if (m_refreshBuffers) {
        [id_cast(MTLBuffer, m_vertexBuffer) release];
        m_vertexBuffer = [
                             id_cast(MTLDevice, m_device)
                             newBufferWithLength: m_vertices.size() * sizeof(float)
                             options: MTLResourceStorageModeShared
                         ];
    }
    void *vertex_ptr = [id_cast(MTLBuffer, m_vertexBuffer) contents];
    memcpy(vertex_ptr, m_vertices.data(), m_vertices.size() * sizeof(float));

    void *matrix_ptr = [id_cast(MTLBuffer, m_matrixBuffer) contents];
    if (matrix_ptr) {
        m_matrix.copyDataTo(static_cast<float *>(matrix_ptr));
    }

    auto encoder = id_cast(MTLRenderCommandEncoder, commandEncoder());

    [encoder setVertexBuffer: id_cast(MTLBuffer, m_vertexBuffer) offset: 0 atIndex: 0];
    [encoder setVertexBuffer: id_cast(MTLBuffer, m_matrixBuffer) offset: 0 atIndex: 1];
    [encoder setFragmentBuffer: id_cast(MTLBuffer, m_colorBuffer) offset: 0 atIndex: 1];
    [encoder setRenderPipelineState: id_cast(MTLRenderPipelineState, m_pipelineBars)];
    [encoder drawPrimitives: MTLPrimitiveTypeTriangle
             vertexStart: 0
             vertexCount: verticesCollected];

    [encoder endEncoding];
}

void RTASeriesNode::renderLines()
{
    if (!m_pipelineLine) {
        return;
    }

    unsigned int vertexCount = m_source->frequencyDomainSize() * 6 * 2;
    if (m_vertices.size() != vertexCount * 5) {
        m_vertices.resize(vertexCount * 5, 0);
        m_refreshBuffers = true;
    }

    float offset = 0;
    Math::EqualLoudnessContour elc;
    switch (m_scale) {
    case RTAPlot::Scale::DBfs:
        offset = 0;
        break;
    case RTAPlot::Scale::SPL:
    case RTAPlot::Scale::Phon:
        offset = absolute_scale_offset;
        break;
    }

    auto addPoint = [ &, this] (auto & i, auto x, auto y, auto dX, auto dY, auto d) {
        m_vertices[i + 0] = x;
        m_vertices[i + 1] = y;

        m_vertices[i + 2] = dX;
        m_vertices[i + 3] = dY;

        m_vertices[i + 4] = d;
        i += 5;
    };

    auto addSegment = [&] (auto & i, auto fromX, auto fromY, auto toX, auto toY) {
        addPoint(i, fromX, fromY, toX, toY, 1.f);
        addPoint(i, fromX, fromY, toX, toY, 1.f);
        addPoint(i, toX, toY, fromX, fromY, -1.f);

        addPoint(i, fromX, fromY, toX, toY, 1.f);
        addPoint(i, toX, toY, fromX, fromY, -1.f);
        addPoint(i, toX, toY, fromX, fromY, -1.f);
    };

    unsigned int j = 0, i;
    float peak;
    for (i = 0; i < m_source->frequencyDomainSize(); ++i) {
        auto value = 20 * log10f(m_source->module(i)) + offset;
        if (m_scale == RTAPlot::Scale::Phon) {
            value = elc.phone(m_source->frequency(i), value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
        }

        addSegment(j,
                   m_source->frequency(i), m_yMin,
                   m_source->frequency(i), value
                  );

        if (m_showPeaks) {
            peak = 10 * log10f(m_source->peakSquared(i)) + offset;
            if (m_scale == RTAPlot::Scale::Phon) {
                peak = elc.phone(m_source->frequency(i), peak + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            }
            addSegment(j,
                       m_source->frequency(i), peak,
                       m_source->frequency(i), peak + 1
                      );
        }
    }

    if (m_refreshBuffers) {
        [id_cast(MTLBuffer, m_vertexBuffer) release];
        m_vertexBuffer = [
                             id_cast(MTLDevice, m_device)
                             newBufferWithLength: m_vertices.size() * sizeof(float)
                             options: MTLResourceStorageModeShared
                         ];
    }
    void *vertex_ptr = [id_cast(MTLBuffer, m_vertexBuffer) contents];
    if (vertex_ptr) {
        memcpy(vertex_ptr, m_vertices.data(), m_vertices.size() * sizeof(float));
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
    [encoder setRenderPipelineState: id_cast(MTLRenderPipelineState, m_pipelineLine)];
    [encoder drawPrimitives: MTLPrimitiveTypeTriangle
             vertexStart: 0
             vertexCount: vertexCount];

    [encoder endEncoding];
}

} // namespace chart
