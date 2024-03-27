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
#include "qmath.h"
#include "bandpass.h"

namespace math {

BandPass::BandPass(float frequency, float q, unsigned sampleRate) :
    m_frequency(frequency), m_q(q), m_sampleRate(sampleRate)
{
    calculate();
}

void BandPass::setFrequency(float newFrequency)
{
    m_frequency = newFrequency;
}

void BandPass::setSampleRate(const unsigned &newSampleRate)
{
    m_sampleRate = newSampleRate;
}

void BandPass::setQ(float newQ)
{
    m_q = newQ;
}

void BandPass::calculate()
{
    float w0 = 2 * M_PI * m_frequency / m_sampleRate;
    float a = std::sin(w0) / (2 * m_q);

    m_b[0] = a;
    m_b[1] = 0;
    m_b[2] = -a;

    m_a[0] = 1 + a;
    m_a[1] = -2 * std::cos(w0);
    m_a[2] = 1 - a;
}

} // namespace math
