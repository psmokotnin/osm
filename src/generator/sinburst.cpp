/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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

#include "sinburst.h"

#include <cmath>

SinBurst::SinBurst(QObject *parent)
    : OutputDevice{parent},
      m_frequency(1000.f),
      m_phase(0.0),
      m_periods(0),
      m_burst(true)
{
    m_name = "SinBurst";
    connect(parent, SIGNAL(frequencyChanged(int)), this, SLOT(setFrequency(int)));
}

Sample SinBurst::sample()
{
    if (!m_sampleRate || !std::isfinite(m_phase)) {
        m_phase = 0;
        return { 0 };
    }
    m_phase += (2.0 * M_PI * static_cast<double>(m_frequency) / m_sampleRate);

    if (m_phase >= 2.0 * M_PI) {
        m_phase -= 2.0 * M_PI;
        m_periods++;

        if ((m_burst && m_periods > 10) || (!m_burst && m_periods > m_frequency / 2)) {
            m_burst = !m_burst;
            m_periods = 0;
        }
    }

    constexpr static float PI10 = M_PI / 10.0;
    Sample output = { m_burst ? m_gain * static_cast<float>(std::sin(m_phase)) * std::sin(m_periods * PI10) / 2.f : 0.f };
    return output;
}

void SinBurst::setFrequency(int f)
{
    m_frequency = static_cast<float>(f);
}
