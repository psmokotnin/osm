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
#include "spectrogramplot.h"

using namespace Chart;

SpectrogramPlot::SpectrogramPlot(Settings *settings, QQuickItem *parent): FrequencyBasedPlot(settings, parent),
    m_min(-90), m_mid(-50), m_max(10), m_active(true)
{
    m_y.configure(AxisType::Linear, 0.f,    4.f,  4);
    setPointsPerOctave(48);
    setFlag(QQuickItem::ItemHasContents);
    m_y.setCentralLabel(m_y.min() - 1.f);
    m_y.setUnit("s");
    connect(this, SIGNAL(pointsPerOctaveChanged(unsigned int)), this, SLOT(update()));
}

void SpectrogramPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Spectrogram")) {
        FrequencyBasedPlot::setSettings(settings);
    }

    setMin(m_settings->reactValue<SpectrogramPlot, int>("dBMin", this, &SpectrogramPlot::minChanged,
                                                        m_min).toInt());
    setMid(m_settings->reactValue<SpectrogramPlot, int>("dBMid", this, &SpectrogramPlot::midChanged,
                                                        m_mid).toInt());
    setMax(m_settings->reactValue<SpectrogramPlot, int>("dBMax", this, &SpectrogramPlot::maxChanged,
                                                        m_max).toInt());
}

void SpectrogramPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    FrequencyBasedPlot::storeSettings();
    m_settings->setValue("type", "Spectrogram");
    m_settings->setValue("dBMin", m_min);
    m_settings->setValue("dBMid", m_mid);
    m_settings->setValue("dBMax", m_max);
}

int SpectrogramPlot::max() const
{
    return m_max;
}

void SpectrogramPlot::setMax(int max)
{
    if (m_max != max) {
        m_max = max;
        emit maxChanged(m_max);
    }
}

bool SpectrogramPlot::active() const
{
    return m_active;
}

void SpectrogramPlot::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        emit activeChanged(m_active);
    }
}

int SpectrogramPlot::mid() const
{
    return m_mid;
}

void SpectrogramPlot::setMid(int mid)
{
    if (m_mid != mid) {
        m_mid = mid;
        emit midChanged(m_mid);
    }
}

int SpectrogramPlot::min() const
{
    return m_min;
}

void SpectrogramPlot::setMin(int min)
{
    if (m_min != min) {
        m_min = min;
        emit minChanged(m_min);
    }
}
