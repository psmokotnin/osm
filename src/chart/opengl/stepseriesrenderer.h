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
#ifndef CHART_STEPSERIESRENDERER_H
#define CHART_STEPSERIESRENDERER_H

#include "xyseriesrenderer.h"
#include "math/windowfunction.h"

namespace chart {

class StepSeriesRenderer : public XYSeriesRenderer
{
public:
    StepSeriesRenderer();
    void init() override;
    void renderSeries() override;
    void synchronize(QQuickFramebufferObject *item) override;

protected:
    virtual void updateMatrix() override;

private:
    int m_matrixUniform, m_widthUniform, m_screenUniform;
    WindowFunction m_window;
    std::vector<float> m_windowed;
    float m_zero;
};

} // namespace chart

#endif // CHART_STEPSERIESRENDERER_H
