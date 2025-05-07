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
#include "stepseriesnode.h"
#include "../stepplot.h"
#include <Metal/Metal.h>

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

StepSeriesNode::StepSeriesNode(QQuickItem *item) : XYSeriesNode(item), m_pipeline(nullptr)
{
}

StepSeriesNode::~StepSeriesNode()
{
    [id_cast(MTLRenderPipelineState, m_pipeline) release];
}

void StepSeriesNode::initRender()
{
    m_pipeline = initLine("lineVertex");
}

void StepSeriesNode::synchronizeSeries()
{
    if (auto *stepPlot = qobject_cast<StepPlot *>(plot())) {
        m_zero = stepPlot->zero();
    }
    synchronizeMatrix();
}

void StepSeriesNode::renderSeries()
{
    if (!m_source->timeDomainSize()) {
        clearRender();
        return;
    }

    unsigned int maxBufferSize = (m_source->timeDomainSize() - 1) * VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE, verticiesCount = 0;
    float *vertex_ptr = vertexBuffer(maxBufferSize);

    float res = 0.f;
    float offsetValue = 0;
    float dcOffset = 0;
    for (unsigned int i = 1; i < m_source->timeDomainSize() / 4; ++i) {
        dcOffset += m_source->impulseValue(i);
    }
    dcOffset /= m_source->timeDomainSize() / 4;

    for (unsigned int i = 0, j = 0; i < m_source->timeDomainSize() - 1; ++i) {
        res += m_source->impulseValue(i) - dcOffset;
        if (m_source->impulseTime(i) < m_zero) {
            offsetValue = res;
        }
        addLineSegment(vertex_ptr, j, verticiesCount,
                       m_source->impulseTime(i), res,
                       m_source->impulseTime(i + 1), res + m_source->impulseValue(i + 1) - dcOffset,
                       1, 1);
    }

    updateMatrix();
    m_matrix.translate(QVector3D(0, -offsetValue, 0));
    encodeLine(m_pipeline, verticiesCount);
}

void StepSeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMin, m_yMax, -1, 1);
}

} // namespace chart
