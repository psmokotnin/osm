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
#ifndef PINKNOISE_H
#define PINKNOISE_H

#include "outputdevice.h"

class PinkNoise : public OutputDevice
{
    Q_OBJECT

public:
    PinkNoise(QObject *parent);

private:

    Sample sample() override;
    unsigned long pseudoRandom();

    const static int MAX_RANDOM_ROWS = 30;
    const static int RANDOM_BITS     = 24;
    const static int RANDOM_SHIFT    = ((sizeof(long) * 8) - PinkNoise::RANDOM_BITS);

    long      m_rows[PinkNoise::MAX_RANDOM_ROWS];
    long      m_runningSum;   // Used to optimize summing of generators.
    int       m_index;        // Incremented each sample.
    int       m_indexMask;    // Index wrapped by ANDing with this mask.
    float     m_scalar;       // Used to scale within range of -1.0 to +1.0
};

#endif // PINKNOISE_H
