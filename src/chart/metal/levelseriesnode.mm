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
#include "levelseriesnode.h"
#include "measurement.h"
#include <Metal/Metal.h>

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

LevelSeriesNode::LevelSeriesNode(QQuickItem *item): XYSeriesNode(item),
    m_timer(), m_history(0)
{
    m_timer.restart();
}

LevelSeriesNode::~LevelSeriesNode()
{
    [id_cast(MTLRenderPipelineState, m_pipeline) release];
}

void LevelSeriesNode::initRender()
{
    m_pipeline = initLine("lineVertex");
}

void LevelSeriesNode::synchronizeSeries()
{
    synchronizeMatrix();
    if (auto *levelPlot = dynamic_cast<LevelPlot *>(plot())) {

        if (m_curve != levelPlot->curve() || m_time != levelPlot->time() || m_mode != levelPlot->mode()) {

            m_curve = levelPlot->curve();
            m_time = levelPlot->time();
            m_mode = levelPlot->mode();
            m_history.clear();
        }
        m_pause = levelPlot->pause();
    }
}

void LevelSeriesNode::renderSeries()
{
    if (!m_source->active()) {
        clearRender();
        return;
    }

    if (m_timer.elapsed() >= Measurement::TIMER_INTERVAL && !m_pause) {
        auto time = static_cast<float>(m_timer.restart()) / 1000.f;
        auto level = m_source->level(m_curve, m_time);
        if (m_mode == LevelPlot::SPL) {
            level += LevelPlot::SPL_OFFSET;
        }
        m_history.push_back( {time, level});
    }

    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.pop_front();
    }

    unsigned int maxBufferSize = MAX_HISTORY_SIZE * VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE, verticiesCount = 0;
    float *vertex_ptr = vertexBuffer(maxBufferSize);

    float time = 0, level = 0;
    unsigned int j = 0, i = 0;
    timePoint lastPoint = {0, 0};

    for (auto point = m_history.crbegin(); point != m_history.crend(); ++point) {
        time -= point->time;
        level = point->value;
        ++i;

        if (lastPoint.time < 0) {
            addLineSegment(vertex_ptr, j, verticiesCount,
                           lastPoint.time, lastPoint.value,
                           time, level,
                           1, 1);
        }
        lastPoint.time = time;
        lastPoint.value = level;
    }

    encodeLine(m_pipeline, verticiesCount);
}

void LevelSeriesNode::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMin, m_yMax, -1, 1);
}

} // namespace chart
