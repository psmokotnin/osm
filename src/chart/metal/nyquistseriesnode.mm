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
#include "nyquistseriesnode.h"
#include "../nyquistplot.h"
#include <Metal/Metal.h>

namespace Chart {

NyquistSeriesNode::NyquistSeriesNode(QQuickItem *item) : XYSeriesNode(item),
    m_pointsPerOctave(0), m_coherenceThreshold(0), m_coherence(false),
    m_pipeline(nullptr)
{
}

NyquistSeriesNode::~NyquistSeriesNode()
{
    [static_cast<id<MTLRenderPipelineState>>(m_pipeline) release];
}

void NyquistSeriesNode::initRender()
{
    m_pipeline = initLine("lineVertex");
}

void NyquistSeriesNode::synchronizeSeries()
{
    synchronizeMatrix();
    if (auto *nyquistPlot = dynamic_cast<NyquistPlot *>(plot())) {
        m_pointsPerOctave    = nyquistPlot->pointsPerOctave();
        m_coherence          = nyquistPlot->coherence();
        m_coherenceThreshold = nyquistPlot->coherenceThreshold();
    }
}

void NyquistSeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMin, m_yMax, -1, 1);
}

const Shared::Source &NyquistSeriesNode::source() const
{
    return m_source;
}

void NyquistSeriesNode::renderSeries()
{
    if (!m_source->frequencyDomainSize()) {
        clearRender();
        return;
    }

    unsigned int maxBufferSize = m_pointsPerOctave * PPO_BUFFER_MUL, j = 0, verticiesCount = 0;
    float *vertex_ptr = vertexBuffer(maxBufferSize);
    NyquistPlot::SplineValue value(0, 0);
    float coherence = 0.f;

    auto accumulate = [ &, this] (const unsigned int &i) {
        value += m_source->phase(i);
        value += m_source->magnitudeRaw(i);
        coherence += m_source->coherence(i);
    };
    auto beforeSpline = [&] (const auto * value, auto, const auto & count) {
        Complex c = value->m_phase / count;
        c /= c.abs();
        c *= value->m_magnitude / count;
        return c;
    };
    auto collected = [&] (const float & f1, const float & f2, const Complex ac[4], const float c[4]) {
        auto points = std::min(MAX_LINE_SPLITF, std::abs(std::round(f2 - f1)));
        float dt = 1.f / points;

        for (float t = 0; t < 1.0;) {
            if (j + 30 > maxBufferSize) {
                qCritical("out of range");
                return;
            }
            auto x1 = ac[0].real + ac[1].real * t + ac[2].real * t * t + ac[3].real * t * t * t;
            auto y1 = ac[0].imag + ac[1].imag * t + ac[2].imag * t * t + ac[3].imag * t * t * t;
            auto c1 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

            t += dt;
            auto x2 = ac[0].real + ac[1].real * t + ac[2].real * t * t + ac[3].real * t * t * t;
            auto y2 = ac[0].imag + ac[1].imag * t + ac[2].imag * t * t + ac[3].imag * t * t * t;
            auto c2 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

            addLineSegment(vertex_ptr, j, verticiesCount,
                           x1, y1,
                           x2, y2,
                           c1, c2);
        }
        coherence = 0.f;
        value.reset();
    };

    iterateForSpline<NyquistPlot::SplineValue, Complex>(m_pointsPerOctave, &value, &coherence, accumulate, collected,
                                                        beforeSpline);
    encodeLine(m_pipeline, verticiesCount);
}

} // namespace chart
