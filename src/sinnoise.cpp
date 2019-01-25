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
#include "sinnoise.h"

SinNoise::SinNoise(QObject *parent) : OutputDevice(parent)
{
    name = "Sin";
    connect(parent, SIGNAL(frequencyChanged(int)), this, SLOT(setFrequency(int)));
}

Sample SinNoise::sample(void)
{
    Sample output;
    sinPhase += (2.0 * M_PI * static_cast<double>(frequency) / sampleRate);
    if (sinPhase >= 2.0 * M_PI)
        sinPhase -= 2.0 * M_PI;

    output.f = m_gain * static_cast<float>(sin(sinPhase));
    return output;
}
void SinNoise::setFrequency(int f)
{
    frequency = static_cast<float>(f);
}
