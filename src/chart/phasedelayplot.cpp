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
#include "phasedelayplot.h"
#include <QtMath>

using namespace Chart;

PhaseDelayPlot::PhaseDelayPlot(Settings *settings, QQuickItem *parent) :
    FrequencyBasedPlot(settings, parent)
{
    m_pointsPerOctave = 12;
    m_y.configure(AxisType::Linear,
                  -50.f * static_cast<float>(M_PI) / 500,  //min
                  50.f  * static_cast<float>(M_PI) / 500,   //max
                  21,     //ticks
                  500.f / static_cast<float>(M_PI)    //scale
                 );

    m_y.setReset(
        -1.f * static_cast<float>(M_PI) / 500,
        40.f * static_cast<float>(M_PI) / 500);
    m_y.reset();
    m_y.setUnit("ms");
    setFlag(QQuickItem::ItemHasContents);
}

void PhaseDelayPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Phase Delay")) {
        FrequencyBasedPlot::setSettings(settings);
    }
}
void PhaseDelayPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    FrequencyBasedPlot::storeSettings();
}
