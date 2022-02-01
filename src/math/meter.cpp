/**

 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#include <cmath>
#include "meter.h"
#include <QtGlobal>

Meter::Meter(unsigned long size) :
    m_size(size),
    m_integrator(0.f),
    m_peak(0.f)
{
}
void Meter::add(const float &data) noexcept
{
    float d = std::pow(data, 2);
    auto p = m_data.pushnpop(d, m_size);
    m_integrator -= m_data.pushnpop(d, m_size);
    m_integrator += d;

    if (qFuzzyCompare(m_peak, p)) {
        m_peak = d;
    }
    m_peak = std::max(m_peak, d);

    while ( m_data.size() > m_size) {
        m_integrator -= m_data.front();
        m_data.pop();
    }
    if (m_integrator < 0.f)
        m_integrator = 0.f;
}
float Meter::value() const noexcept
{
    if (m_data.size() == 0)
        return std::numeric_limits<float>::min();

    return m_integrator / m_data.size();
}
float Meter::dB() const noexcept
{
    // 20log(sqrt(v)) = 10log(v)
    return 10.f * std::log10(value());
}

float Meter::peakSquared() const noexcept
{
    return m_peak;
}

float Meter::peakdB() const noexcept
{
    return 10.f * std::log10(m_peak);;
}

void Meter::reset() noexcept
{
    while (m_data.size()) {
        m_data.pop();
    }
    m_integrator = 0.f;
    m_peak = 0.f;
}
