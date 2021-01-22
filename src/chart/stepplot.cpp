/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

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
#include "stepplot.h"
#include "stepseriesrenderer.h"
using namespace Fftchart;

StepPlot::StepPlot(Settings *settings, QQuickItem *parent) : XYPlot(settings, parent)
{
    m_x.configure(AxisType::Linear, -20.0, 20.0, 41);
    m_x.setMin(-5.f);
    m_x.setMax(5.f);
    m_y.configure(AxisType::Linear, -2.0, 2.0, 21);
    m_y.setMin(-1.f);
    m_y.setMax(1.f);
    setFlag(QQuickItem::ItemHasContents);
}

SeriesFBO *StepPlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, [](){return new StepSeriesRenderer();}, this);
}

void StepPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Impulse")) {
        XYPlot::setSettings(settings);
    }
}

void StepPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
}
