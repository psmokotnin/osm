/**
 *  OSM
 *  Copyright (C) 2025  Pavel Smokotnin

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

#include "data.h"

namespace Abstract {

Data::Data()  = default;
Data::~Data() = default;

unsigned int Data::size() const noexcept
{
    return m_dataLength;
}

float Data::frequency(unsigned int i) const noexcept
{
    if (i < m_dataLength) {
        return m_ftdata[i].frequency;
    }
    return 0;
}

float Data::module (unsigned int i) const noexcept {
    if (i < m_dataLength)
    {
        return m_ftdata[i].module;
    }
    return 0;
}

float Data::magnitude(unsigned int i) const noexcept
{
    if (i < m_dataLength) {
        return 20.f * log10f(m_ftdata[i].magnitude);
    }
    return 0;
}

float Data::magnitudeRaw(unsigned int i) const noexcept
{
    if (i < m_dataLength) {
        return m_ftdata[i].magnitude;
    }
    return 0;
}

complex Data::phase(unsigned int i) const noexcept
{
    if (i < m_dataLength) {
        return m_ftdata[i].phase;
    }
    return 0;
}

float Data::coherence(unsigned int i) const noexcept
{
    if (i < m_dataLength) {
        return m_ftdata[i].coherence;
    }
    return 0;
}

float Data::peakSquared(unsigned int i) const noexcept
{
    if (i < m_dataLength) {
        return m_ftdata[i].peakSquared;
    }
    return 0;
}

float Data::crestFactor(unsigned int i) const noexcept
{
    if (i < m_dataLength) {
        return 10.f * std::log10(m_ftdata[i].peakSquared / m_ftdata[i].meanSquared);
    }
    return -INFINITY;
}

unsigned int Data::impulseSize() const noexcept
{
    return m_deconvolutionSize;
}

float Data::impulseTime(unsigned int i) const noexcept
{
    if (i < m_deconvolutionSize) {
        return m_impulseData[i].time;
    }
    return 0;
}

float Data::impulseValue(unsigned int i) const noexcept
{
    if (i < m_deconvolutionSize) {
        return m_impulseData[i].value.real;
    }
    return 0;
}

float Data::level(const Weighting::Curve curve, const Meter::Time time) const
{
    if (m_levelsData.m_data.find({curve, time}) == m_levelsData.m_data.end()) {
        Q_ASSERT(false);
        return 0;
    }
    return m_levelsData.m_data.at({curve, time});
}

float Data::peak(const Weighting::Curve curve, const Meter::Time time) const
{
    if (m_levelsData.m_data.find({curve, time}) == m_levelsData.m_data.end()) {
        Q_ASSERT(false);
        return 0;
    }
    return m_levelsData.m_data.at({curve, time});//TODO: m_peakData ??
}

float Data::referenceLevel() const
{
    return m_levelsData.m_referenceLevel;
}

void Data::copy(FTData *dataDist, TimeData *timeDist)
{
    if (dataDist) {
        std::copy_n(m_ftdata.data(), size(), dataDist);
    }
    if (timeDist) {
        std::copy_n(m_impulseData.data(), impulseSize(), timeDist);
    }
}

void Data::copyFrom(size_t dataSize, size_t timeSize, FTData *dataSrc, TimeData *timeSrc)
{
    m_dataLength = dataSize;
    m_deconvolutionSize = timeSize;
    m_ftdata.resize(m_dataLength);
    m_impulseData.resize(m_deconvolutionSize);

    std::copy_n(dataSrc, size(), m_ftdata.data());
    std::copy_n(timeSrc, impulseSize(), m_impulseData.data());
}

}
