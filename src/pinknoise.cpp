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
#include "pinknoise.h"

PinkNoise::PinkNoise(QObject *parent) : OutputDevice(parent),
    m_rows()
{
    m_name = "Pink";

    long pmax;
    int numRows = 12;

    m_index = 0;
    m_indexMask = (1 << numRows) - 1;
    /* Calculate maximum possible signed random value. Extra 1 for white noise always added. */
    pmax = (numRows + 1) * (1 << (RANDOM_BITS - 1));
    m_scalar = 1.0f / pmax;

    /* Initialize rows. */
    for (int i = 0; i < numRows; i++)
        m_rows[i] = 0;

    m_runningSum = 0;
}
Sample PinkNoise::sample()
{
    long newRandom;
    long sum;

    /* Increment and mask index. */
    m_index = (m_index + 1) & m_indexMask;

    /* If index is zero, don't update any random values. */
    if (m_index != 0 ) {
        /* Determine how many trailing zeros in PinkIndex. */
        /* This algorithm will hang if n==0 so test first. */
        int numZeros = 0;
        int n = m_index;
        while ((n & 1) == 0) {
            n = n >> 1;
            numZeros++;
        }

        /* Replace the indexed ROWS random value.
         * Subtract and add back to RunningSum instead of adding all the random
         * values together. Only one changes each time.
         */
        m_runningSum -= m_rows[numZeros];
        newRandom = (static_cast<long>(pseudoRandom())) >> RANDOM_SHIFT;
        m_runningSum += newRandom;
        m_rows[numZeros] = newRandom;
    }

    /* Add extra white noise value. */
    newRandom = (static_cast<long>(pseudoRandom())) >> RANDOM_SHIFT;
    sum = m_runningSum + newRandom;

    /* Scale to range of -1.0 to 0.9999. */
    Sample output = {m_gain *m_scalar * sum};
    return output;
}

/* Calculate pseudo-random 32 bit number based on linear congruential method. */
unsigned long PinkNoise::pseudoRandom()
{
    static unsigned long randSeed = 22222;  /* Change this for different random sequences. */
    randSeed = (randSeed * 196314165) + 907633515;
    return randSeed;
}
