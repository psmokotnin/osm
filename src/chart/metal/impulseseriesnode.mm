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
#include "impulseseriesnode.h"
#include "../stepplot.h"
#include <Metal/Metal.h>

namespace chart {

#define id_cast(T, t) static_cast<id<T>>(t)

ImpulseSeriesNode::ImpulseSeriesNode(QQuickItem *item) : XYSeriesNode(item),
    m_pipeline(nullptr)
{

}

ImpulseSeriesNode::~ImpulseSeriesNode()
{
    [id_cast(MTLRenderPipelineState, m_pipeline) release];
}

void ImpulseSeriesNode::initRender()
{
    m_pipeline = initLine("lineVertex");
}

void ImpulseSeriesNode::synchronizeSeries()
{
    synchronizeMatrix();
}

void ImpulseSeriesNode::renderSeries()
{
    if (!m_source->impulseSize()) {
        clearRender();
        return;
    }

    unsigned int maxBufferSize = (m_source->impulseSize() - 1) * VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE, verticiesCount = 0;
    float *vertex_ptr = vertexBuffer(maxBufferSize);

    float dcOffset =  m_source->impulseValue(0);
    for (unsigned int i = 0, j = 0; i < m_source->impulseSize() - 1; ++i) {
        addLineSegment(vertex_ptr, j, verticiesCount,
                       m_source->impulseTime(i), m_source->impulseValue(i) - dcOffset,
                       m_source->impulseTime(i + 1),  m_source->impulseValue(i + 1) - dcOffset,
                       1, 1);
    }

    encodeLine(m_pipeline, verticiesCount);
}

void ImpulseSeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMin, m_yMax, -1, 1);
}

} // namespace chart
