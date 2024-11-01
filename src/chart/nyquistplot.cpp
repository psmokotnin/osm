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
#include "nyquistplot.h"

namespace Chart {

NyquistPlot::NyquistPlot(Settings *settings, QQuickItem *parent) : FrequencyBasedPlot(settings, parent)
{
    m_x.configure(AxisType::Linear, -5.0, 5.0, 41);
    m_x.setReset(-1.1f, 1.1f);
    m_x.reset();
    m_y.configure(AxisType::Linear, -5.0, 5.0, 41);
    m_y.setReset(-1.1f, 1.1f);
    m_y.reset();
    m_x.setUnit("");
    m_y.setUnit("");
    setFlag(QQuickItem::ItemHasContents);
}

void NyquistPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Nyquist")) {
        XYPlot::setSettings(settings);
    }
}

void NyquistPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
}

} // namespace chart
