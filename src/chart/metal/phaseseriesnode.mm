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
#include "phaseseriesnode.h"
#include "../phaseplot.h"
#include <Metal/Metal.h>

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

PhaseSeriesNode::PhaseSeriesNode(QQuickItem *item) : XYSeriesNode(item),
    m_pointsPerOctave(0), m_coherenceThreshold(0), m_coherence(false),
    m_rotate(0), m_pipeline(nullptr)
{

}

PhaseSeriesNode::~PhaseSeriesNode()
{
    [id_cast(MTLRenderPipelineState, m_pipeline) release];
}

void PhaseSeriesNode::initRender()
{
    m_pipeline = initLine();
}

void PhaseSeriesNode::synchronizeSeries()
{
    synchronizeMatrix();
    if (auto *phasePlot = dynamic_cast<PhasePlot *>(plot())) {
        m_pointsPerOctave = phasePlot->pointsPerOctave();
        m_coherence = phasePlot->coherence();
        constexpr float pk = static_cast<float>(-M_PI / 180.0);
        m_rotate = phasePlot->rotate() *  pk;
        m_coherenceThreshold = phasePlot->coherenceThreshold();
    }
}

void PhaseSeriesNode::renderSeries()
{
    if (!m_source->frequencyDomainSize()) {
        clearRender();
        return;
    }

    unsigned int maxBufferSize = m_pointsPerOctave * PPO_BUFFER_MUL, j = 0, verticiesCount = 0;
    float *vertex_ptr = vertexBuffer(maxBufferSize);

    Complex value(0);
    float coherence = 0.f;

    float xadd, xmul;
    xadd = -1.0f * logf(m_xMin);
    xmul = width() / logf(m_xMax / m_xMin);

    auto accumulate = [&value, &coherence, this] (const unsigned int &i) {
        value += m_source->phase(i);
        coherence += m_source->coherence(i);
    };
    auto beforeSpline = [this] (const auto * value, auto, const auto & count) {
        return value->rotate(m_rotate) / count;
    };
    auto collected = [ & ] (const float & f1, const float & f2, const Complex ac[4], const float c[4]) {
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
            auto c1 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);
            t += dt;
            auto x2 = f1 * std::pow(f2 / f1, t);
            auto v2 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;
            auto c2 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

            float alpha = std::atan2f(v1.imag, v1.real);
            float beta = std::atan2f(v2.imag, v2.real);
            if (std::abs(alpha - beta) > M_PI) {
                if (alpha > 0) {
                    alpha -= 2 * M_PI;
                } else {
                    alpha += 2 * M_PI;
                }
            }

            addLineSegment(vertex_ptr, j, verticiesCount,
                           x1, alpha,
                           x2, beta,
                           c1, c2);
        }

        value = 0.f;
        coherence = 0.f;
    };
    iterateForSpline<Complex, Complex>(m_pointsPerOctave, &value, &coherence, accumulate, collected, beforeSpline);

    encodeLine(m_pipeline, verticiesCount);
}

void PhaseSeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMin, m_yMax, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), 0);
}

const Shared::Source &PhaseSeriesNode::source() const
{
    return m_source;
}

} // namespace chart
