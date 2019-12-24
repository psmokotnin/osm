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
#include "magnitudeplot.h"
#include "magnitudeseriesrenderer.h"

using namespace Fftchart;

MagnitudePlot::MagnitudePlot(Settings *settings, QQuickItem *parent): FrequencyBasedPlot(settings, parent)
{
    x.configure(AxisType::logarithmic, 20.f, 20000.f);
    x.setISOLabels();
    y.configure(AxisType::linear, -18.f, 18.f,  13);
    setFlag(QQuickItem::ItemHasContents);
}
SeriesFBO* MagnitudePlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, [](){return new MagnitudeSeriesRenderer();}, this);
}
void MagnitudePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Magnitude"))
    {
        FrequencyBasedPlot::setSettings(settings);
    }
}
void MagnitudePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    FrequencyBasedPlot::storeSettings();
}
