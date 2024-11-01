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
#ifndef CHART_LEVELSERIESRENDERER_H
#define CHART_LEVELSERIESRENDERER_H

#include <deque>
#include <QElapsedTimer>

#include "xyseriesrenderer.h"
#include "chart/levelplot.h"
#include "math/weighting.h"
#include "math/meter.h"

namespace Chart {

class LevelSeriesRenderer : public XYSeriesRenderer
{
public:
    LevelSeriesRenderer();
    void init() override;
    void renderSeries() override;
    virtual void synchronize(QQuickFramebufferObject *item) override;

protected:
    virtual void updateMatrix() override;

private:
    int m_widthUniform, m_screenUniform;

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

#endif // CHART_LEVELSERIESRENDERER_H
