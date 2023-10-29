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
#ifndef MATH_NOTCH_H
#define MATH_NOTCH_H

#include "math/biquad.h"

namespace math {

class Notch : public math::BiQuad
{
public:
    Notch(float frequency = 0, float q = 1, unsigned sampleRate = 0);
    virtual ~Notch() = default;

    void setFrequency(float newFrequency);
    void setSampleRate(const unsigned &newSampleRate);

    void calculate();

    void setQ(float newQ);

private:
    float m_frequency;
    float m_q;
    unsigned m_sampleRate;
};

} // namespace math

#endif // MATH_NOTCH_H
