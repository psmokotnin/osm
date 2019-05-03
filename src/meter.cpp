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

Meter::Meter(unsigned long size) :
    m_size(size),
    m_integrator(0.f)
{
}
void Meter::add(const float &data) noexcept
{
    float d = std::abs(data);
    m_integrator -= m_data.pushnpop(d, m_size);
    m_integrator += d;
    while( m_data.size() > m_size) {
        m_integrator -= m_data.front();
        m_data.pop();
    }
    if (m_integrator < 0.f)
        m_integrator = 0.f;
}
float Meter::value() const noexcept
{
    if (m_data.size() == 0)
        return -INFINITY;

    return 20.f * std::log10(m_integrator / m_data.size());//dBV
}
void Meter::reset() noexcept
{
    while (m_data.size()) {
        m_data.pop();
    }
    m_integrator = 0.f;
}
