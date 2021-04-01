/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#include "impulseplot.h"
#include "seriesfbo.h"
#include "impulseseriesrenderer.h"

using namespace Fftchart;

ImpulsePlot::ImpulsePlot(Settings *settings, QQuickItem *parent): XYPlot(settings, parent)
{
    m_x.configure(AxisType::Linear, -20.0, 20.0, 41);
    m_x.setReset(-5.f, 5.f);
    m_x.reset();
    m_y.configure(AxisType::Linear, -2.0, 2.0, 21);
    m_y.setReset(-1.f, 1.f);
    m_y.reset();
    m_x.setUnit("ms");
    m_y.setUnit("");
    setFlag(QQuickItem::ItemHasContents);
}
SeriesFBO *ImpulsePlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, []() {
        return new ImpulseSeriesRenderer();
    }, this);
}
void ImpulsePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Impulse")) {
        XYPlot::setSettings(settings);
    }
}
void ImpulsePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
}
