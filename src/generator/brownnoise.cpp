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
#include "brownnoise.h"

BrownNoise::BrownNoise(QObject *parent)
    : OutputDevice{parent}, m_lastSample{0}
{
    m_name = "Brown";
}

Sample BrownNoise::sample()
{
    constexpr float r1 = 0.97f; // white before ~200Hz
    static const float r2 = sqrt(1 - r1 * r1);

    auto w = static_cast<float>(m_generator.bounded(2.0) - 1.0);
    Sample s = { m_lastSample.f *r1 + r2 * w };
    m_lastSample = s;
    s.f *= m_gain / 8; // -18dB
    return s;
}
