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
#include "crestfactorseriesnode.h"
#include <Metal/Metal.h>

namespace Chart {

CrestFactorSeriesNode::CrestFactorSeriesNode(QQuickItem *item) : XYSeriesNode(item),
    m_pointsPerOctave(0), m_pipeline(nullptr)
{

}

CrestFactorSeriesNode::~CrestFactorSeriesNode()
{
    [static_cast<id<MTLRenderPipelineState>>(m_pipeline) release];
}

void CrestFactorSeriesNode::initRender()
{
    m_pipeline = initLine();
}

void CrestFactorSeriesNode::synchronizeSeries()
{
    synchronizeMatrix();
    if (auto *crestFactorPlot = dynamic_cast<CrestFactorPlot *>(plot())) {
        m_pointsPerOctave = crestFactorPlot->pointsPerOctave();
    }
}

void CrestFactorSeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMin, m_yMax, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), 0);
}

const Shared::Source &CrestFactorSeriesNode::source() const
{
    return m_source;
}

void CrestFactorSeriesNode::renderSeries()
{
    if (!m_source->frequencyDomainSize()) {
        clearRender();
        return;
    }

    unsigned int maxBufferSize = m_pointsPerOctave * PPO_BUFFER_MUL, j = 0, verticiesCount = 0;
    float *vertex_ptr = vertexBuffer(maxBufferSize);
    float value = 0.f, coherence = 0.f;
    float xadd, xmul;
    xadd = -1.0f * logf(m_xMin);
    xmul = width() / logf(m_xMax / m_xMin);

    auto accumulate = [this, &value] (const unsigned int &i) {
        value += m_source->crestFactor(i);
    };
    auto collected = [ &] (const float & f1, const float & f2, const float * ac, const float *) {

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
            t += dt;
            auto x2 = f1 * std::pow(f2 / f1, t);
            auto v2 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;

            addLineSegment(vertex_ptr, j, verticiesCount,
                           x1, v1,
                           x2, v2,
                           1.f, 1.f);
        }

        value = 0.f;
    };

    iterateForSpline<float, float>(m_pointsPerOctave, &value, &coherence, accumulate, collected);
    encodeLine(m_pipeline, verticiesCount);
}

} // namespace chart
