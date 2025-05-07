/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#include "phasedelayseriesnode.h"
#include "../phasedelayplot.h"
#include <Metal/Metal.h>

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

PhaseDelaySeriesNode::PhaseDelaySeriesNode(QQuickItem *item) : XYSeriesNode(item),
    m_pointsPerOctave(0), m_coherenceThreshold(0), m_coherence(false),
    m_pipeline(nullptr)
{
}

PhaseDelaySeriesNode::~PhaseDelaySeriesNode()
{
    [id_cast(MTLRenderPipelineState, m_pipeline) release];
}

void PhaseDelaySeriesNode::initRender()
{
    m_pipeline = initLine();
}

void PhaseDelaySeriesNode::synchronizeSeries()
{
    synchronizeMatrix();
    if (auto *groupDelayPlot = dynamic_cast<PhaseDelayPlot *>(plot())) {
        m_pointsPerOctave = groupDelayPlot->pointsPerOctave();
        m_coherence = groupDelayPlot->coherence();
        m_coherenceThreshold = groupDelayPlot->coherenceThreshold();
    }
}

void PhaseDelaySeriesNode::renderSeries()
{
    if (!m_source->frequencyDomainSize()) {
        clearRender();
        return;
    }

    unsigned int maxBufferSize = m_pointsPerOctave * PPO_BUFFER_MUL, j = 0, verticiesCount = 0;
    float *vertex_ptr = vertexBuffer(maxBufferSize);
    float value(0), lastValue(0);
    float coherence = 0.f;
    int periods = 0;

    float xadd, xmul;
    xadd = -1.0f * logf(m_xMin);
    xmul = width() / logf(m_xMax / m_xMin);

    auto accumulate = [ &, this] (const unsigned int &i) {
        auto v = m_source->phase(i).arg() + periods * 2.0 * M_PI;
        if (std::abs(lastValue - v) > M_PI) {
            periods += (lastValue - v) > 0 ? 1 : -1;
            v = m_source->phase(i).arg() + periods * 2.0 * M_PI;
        }
        value +=  v;
        lastValue = v;
        coherence += m_source->coherence(i);
    };
    auto beforeSpline = [&] (const auto * value, auto, const auto & count) {
        return (*value) / count;
    };
    auto collected = [&] (const float & f1, const float & f2, const float ac[4], const float c[4]) {
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;
        auto points = std::min(MAX_LINE_SPLITF, std::abs(std::round(fx2 - fx1)));
        float dt = 1.f / points;

        for (float t = 0; t < 1.0;) {
            if (j + 30 > maxBufferSize) {
                qCritical("out of range");
                return;
            }
            auto x1 = f1 * std::pow(f2 / f1, t);
            auto v1 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;
            v1 /= x1;
            auto c1 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

            t += dt;
            auto x2 = f1 * std::pow(f2 / f1, t);
            auto v2 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;
            v2 /= x2;
            auto c2 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

            addLineSegment(vertex_ptr, j, verticiesCount,
                           x1, -v1,
                           x2, -v2,
                           c1, c2);
        }
        coherence = 0.f;
        value = 0.f;
    };

    iterateForSpline<float, float>(m_pointsPerOctave, &value, &coherence, accumulate, collected, beforeSpline);
    encodeLine(m_pipeline, verticiesCount);
}

void PhaseDelaySeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMin, m_yMax, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), 0);
}

const Shared::Source &PhaseDelaySeriesNode::source() const
{
    return m_source;
}

} // namespace chart
