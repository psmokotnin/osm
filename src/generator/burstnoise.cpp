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
#include "burstnoise.h"

BurstNoise::BurstNoise(QObject *parent)
    : OutputDevice(parent), m_state(0), m_generator()
{
    m_name = "Burst";
}
Sample BurstNoise::sample()
{
    if (m_generator.bounded(2.0) > 1.999) {
        m_state = !m_state;
    }
    Sample s = {m_gain * m_state};
    return s;
}

