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

unsigned int Data::frequencyDomainSize() const noexcept
{
    return m_ftdata.size();
}

void Data::setFrequencyDomainSize(unsigned int size)
{
    m_ftdata.resize(size);
}

unsigned int Data::timeDomainSize() const noexcept
{
    return m_impulseData.size();
}

void Data::setTimeDomainSize(unsigned int size)
{
    m_impulseData.resize(size);
}

void Data::lock()
{
    m_dataMutex.lock();
}

void Data::unlock()
{
    m_dataMutex.unlock();
}

float Data::frequency(unsigned int i) const noexcept
{
    if (i < frequencyDomainSize()) {
        return m_ftdata[i].frequency;
    }
    return 0;
}

float Data::module (unsigned int i) const noexcept {
    if (i < frequencyDomainSize())
    {
        return m_ftdata[i].module;
    }
    return 0;
}

float Data::magnitude(unsigned int i) const noexcept
{
    if (i < frequencyDomainSize()) {
        return 20.f * log10f(m_ftdata[i].magnitude);
    }
    return 0;
}

float Data::magnitudeRaw(unsigned int i) const noexcept
{
    if (i < frequencyDomainSize()) {
        return m_ftdata[i].magnitude;
    }
    return 0;
}

Complex Data::phase(unsigned int i) const noexcept
{
    if (i < frequencyDomainSize()) {
        return m_ftdata[i].phase;
    }
    return 0;
}

float Data::coherence(unsigned int i) const noexcept
{
    if (i < frequencyDomainSize()) {
        return m_ftdata[i].coherence;
    }
    return 0;
}

float Data::peakSquared(unsigned int i) const noexcept
{
    if (i < frequencyDomainSize()) {
        return m_ftdata[i].peakSquared;
    }
    return 0;
}

float Data::crestFactor(unsigned int i) const noexcept
{
    if (i < frequencyDomainSize()) {
        return 10.f * std::log10(m_ftdata[i].peakSquared / m_ftdata[i].meanSquared);
    }
    return -INFINITY;
}

float Data::impulseTime(unsigned int i) const noexcept
{
    if (i < timeDomainSize()) {
        return m_impulseData[i].time;
    }
    return 0;
}

float Data::impulseValue(unsigned int i) const noexcept
{
    if (i < timeDomainSize()) {
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

void Data::copyTo(Data &dist) const
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    dist.lock();

    dist.setFrequencyDomainSize(frequencyDomainSize());
    dist.setTimeDomainSize(timeDomainSize());
    std::copy_n(m_ftdata.data(), frequencyDomainSize(), dist.m_ftdata.data());
    std::copy_n(m_impulseData.data(), timeDomainSize(), dist.m_impulseData.data());

    dist.unlock();
}

void Data::setFrequencyDomainData(std::vector<FTData> &&data)
{
    m_ftdata = data;
}

void Data::setTimeDomainData(std::vector<TimeData> &&data)
{
    m_impulseData = data;
}

}
