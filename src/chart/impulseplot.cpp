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

using namespace Chart;

ImpulsePlot::ImpulsePlot(Settings *settings, QQuickItem *parent): XYPlot(settings, parent), m_mode(Log)
{
    setMode(Linear);
    setFlag(QQuickItem::ItemHasContents);
}

void ImpulsePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Impulse")) {
        XYPlot::setSettings(settings);

        setMode(m_settings->reactValue<ImpulsePlot, ImpulsePlot::Mode>("mode", this, &ImpulsePlot::modeChanged,
                                                                       m_mode).toInt());
        setNormalized(
            m_settings->reactValue<ImpulsePlot, bool>("normalized", this, &ImpulsePlot::normalizedChanged, m_normalized).toBool());
    }
}
void ImpulsePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
    m_settings->setValue("mode", m_mode);
    m_settings->setValue("normalized", m_normalized);
}

ImpulsePlot::Mode ImpulsePlot::mode() const
{
    return m_mode;
}

void ImpulsePlot::setMode(const Mode &mode)
{
    if (m_mode != mode) {
        m_mode = mode;

        switch (m_mode) {
        case Linear:
            m_x.configure(AxisType::Linear, -500.0, 500.0, 1001);
            m_x.setReset(-5.f, 5.f);
            m_x.setUnit("ms");
            m_x.reset();

            m_y.configure(AxisType::Linear, -2.0, 2.0, 21);
            m_y.setReset(-1.f, 1.f);
            m_y.setUnit("");
            m_y.reset();
            break;

        case Log:
            m_x.configure(AxisType::Linear, -2000.0, 20000.0, 401);
            m_x.setReset(-100.f, 900.f);
            m_x.setUnit("ms");
            m_x.reset();

            m_y.configure(AxisType::Linear, -140.f, 40.f,  15);
            m_y.setReset(-100.f, 0.f);
            m_y.setUnit("dB");
            m_y.reset();
            break;
        }
        update();
        emit modeChanged(m_mode);
    }
}

void ImpulsePlot::setMode(const int &mode)
{
    setMode(static_cast<Mode>(mode));
}

bool ImpulsePlot::normalized() const
{
    return m_normalized;
}

void ImpulsePlot::setNormalized(bool newNormalized)
{
    if (m_normalized == newNormalized)
        return;
    m_normalized = newNormalized;
    update();
    emit normalizedChanged(m_normalized);
}
