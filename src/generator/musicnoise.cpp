/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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

#include "musicnoise.h"
#include <math.h>

MusicNoise::MusicNoise(QObject *parent) : OutputDevice(parent)
{
    m_name = "Music-Noise";

    if (!m_48.load(":/audio/musicnoise48.wav")) {
        qDebug() << "can't load Music-Noise 48";
    }
    if (!m_96.load(":/audio/musicnoise96.wav")) {
        qDebug() << "can't load Music-Noise 96";
    }
}

Sample MusicNoise::sample()
{
    Sample s;

    switch (m_sampleRate) {
    case 48000:
        s.f = m_gain * m_48.nextSample(true);
        break;
    case 96000:
        s.f = m_gain * m_96.nextSample(true);
        break;
    default:
        s.f = NAN;
    }

    return s;
}
