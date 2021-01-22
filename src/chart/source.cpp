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
#include <algorithm>
#include "source.h"

using namespace Fftchart;
Source::Source(QObject *parent) : QObject(parent),
    m_ftdata(nullptr),
    m_impulseData(nullptr),
    m_dataLength(0),
    m_deconvolutionSize(0),
    m_active(false)
{
}
void Source::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        emit activeChanged();
    }
}
void Source::setName(QString name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
    }
}
void Source::setColor(QColor color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}
void Source::setGlobalColor(int globalValue)
{
    if (globalValue < 19) {
        m_color = Qt::GlobalColor(globalValue);
        emit colorChanged(m_color);
    }
}
const float &Source::frequency(unsigned int i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].frequency;
}
const float &Source::module(unsigned int i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].module;
}
float Source::magnitude(unsigned int i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return 20.f * log10f(m_ftdata[i].magnitude);
}
const float &Source::magnitudeRaw(unsigned int i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].magnitude;
}
const complex &Source::phase(unsigned int i) const noexcept
{
    if (i <= m_dataLength)
        return m_ftdata[i].phase;

    return m_ftdata[0].phase;
}
const float &Source::coherence(unsigned int i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;

    return m_ftdata[i].coherence;
}
const float &Source::impulseTime(unsigned int i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return m_zero;
    return m_impulseData[i].time;
}
const float &Source::impulseValue(unsigned int i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return m_zero;
    return m_impulseData[i].value.real;
}
void Source::copy(FTData *dataDist, TimeData *timeDist)
{
    std::copy_n(m_ftdata, size(), dataDist);
    std::copy_n(m_impulseData, impulseSize(), timeDist);
}

void Source::copyFrom(size_t dataSize, size_t timeSize, Source::FTData *dataSrc, Source::TimeData *timeSrc)
{
    if (m_ftdata) {
        delete[] m_ftdata;
    }
    if (m_impulseData) {
        delete[] m_impulseData;
    }

    m_dataLength = dataSize;
    m_deconvolutionSize = timeSize;
    m_ftdata = new FTData[m_dataLength];
    m_impulseData = new TimeData[m_deconvolutionSize];

    std::copy_n(dataSrc, size(), m_ftdata);
    std::copy_n(timeSrc, impulseSize(), m_impulseData);
}
