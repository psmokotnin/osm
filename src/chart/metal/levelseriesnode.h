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
#ifndef CHART_LEVELSERIESNODE_H
#define CHART_LEVELSERIESNODE_H

#include <deque>
#include <QElapsedTimer>

#include "xyseriesnode.h"
#include "chart/levelplot.h"

namespace Chart {

class LevelSeriesNode : public XYSeriesNode
{
    Q_OBJECT

public:
    LevelSeriesNode(QQuickItem *item);
    ~LevelSeriesNode();

protected:
    void initRender() override;
    void synchronizeSeries() override;
    void renderSeries() override;
    void updateMatrix() override;

private:
    //! MTLRenderPipelineState
    void *m_pipeline;

    Weighting::Curve m_curve;
    Meter::Time m_time;
    LevelPlot::Mode m_mode;
    bool m_pause;

    QElapsedTimer m_timer;

    struct timePoint {
        float time;
        float value;
    };
    static const unsigned int MAX_HISTORY_SIZE = 750;
    std::deque<timePoint> m_history;
};

} // namespace chart

#endif // CHART_LEVELSERIESNODE_H
