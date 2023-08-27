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

#ifndef MUSICNOISE_H
#define MUSICNOISE_H

#include "common/wavfile.h"
#include "outputdevice.h"

class MusicNoise : public OutputDevice
{
    Q_OBJECT

public:
    explicit MusicNoise(QObject *parent = nullptr);
    Sample sample() final;

private:
    WavFile m_48;
    WavFile m_96;
};

#endif // MUSICNOISE_H
