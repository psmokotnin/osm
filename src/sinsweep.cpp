/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#include "sinsweep.h"
#include "generatorthread.h"
#include <QtMath>

SinSweep::SinSweep(GeneratorThread *parent) : OutputDevice(parent),
    m_phase(0.0),
    m_start(20.f),
    m_end(20000.f),
    m_duration(1.f)
{
    m_name = "SineSweep";
    m_frequency = m_start;

    connect(parent, &GeneratorThread::enabledChanged, this,  &SinSweep::enabledChanged);
    connect(parent, &GeneratorThread::startFrequencyChanged, this, &SinSweep::setStart);
    connect(parent, &GeneratorThread::endFrequencyChanged, this, &SinSweep::setEnd);
    connect(parent, &GeneratorThread::durationChanged, this, &SinSweep::setDuration);
}

Sample SinSweep::sample()
{
    auto T = m_duration;
    auto t = m_phase;
    float phase = 2.f * M_PI * m_start * T / std::log(m_end / m_start);
    phase *= std::exp(std::log(m_end / m_start) * t / T) - 1;

    m_phase += 1.0 / m_sampleRate;
    if (m_phase >= m_duration) {
        m_phase = 0;
    }

    return {m_gain *std::sin(phase)};
}

void SinSweep::setEnd(int end)
{

    m_end = static_cast<float>(end);
    m_frequency = m_start;
}

void SinSweep::setFrequency(int f)
{
    m_frequency = static_cast<float>(f);
}

void SinSweep::setDuration(float duration)
{
    m_duration = duration;
}

void SinSweep::enabledChanged(bool)
{
    m_phase = 0;
}

void SinSweep::setStart(int start)
{
    m_start = static_cast<float>(start);
    m_frequency = m_start;
}

