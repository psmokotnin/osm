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

#ifndef DATA_H
#define DATA_H

#include <mutex>
#include <unordered_map>
#include <vector>

#include "abstract/levelsdata.h"
#include "math/complex.h"

namespace Abstract {

struct Data {

    explicit Data();
    virtual ~Data();

    struct FTData {
        float   frequency   = 0;
        float   module      = 0;
        float   magnitude   = 0;
        Complex phase       = 0;
        float   coherence   = 0;
        float   peakSquared = 0;
        float   meanSquared = NAN;
    };

    struct TimeData {
        float   time  = 0; //ms
        Complex value = 0;
    };


    void            lock();
    void            unlock();

    unsigned int    frequencyDomainSize() const noexcept;
    void            setFrequencyDomainSize(unsigned int size);
    float           frequency(   unsigned int i) const noexcept;
    virtual float   module(      unsigned int i) const noexcept;
    virtual float   magnitude(   unsigned int i) const noexcept;
    virtual float   magnitudeRaw(unsigned int i) const noexcept;
    virtual Complex phase(       unsigned int i) const noexcept;
    virtual float   coherence(   unsigned int i) const noexcept;
    virtual float   peakSquared( unsigned int i) const noexcept;
    virtual float   crestFactor( unsigned int i) const noexcept;

    unsigned int    timeDomainSize() const noexcept;
    void            setTimeDomainSize(unsigned int size);
    virtual float   impulseTime( unsigned int i) const noexcept;
    virtual float   impulseValue(unsigned int i) const noexcept;

    virtual float   level(const Weighting::Curve curve = Weighting::Z, const Meter::Time time = Meter::Fast) const;
    virtual float   peak( const Weighting::Curve curve = Weighting::Z, const Meter::Time time = Meter::Fast) const;
    virtual float   referenceLevel() const;

    void            copyTo(Data &dist) const;
    void            setFrequencyDomainData(std::vector<FTData> &&data);
    void            setTimeDomainData(std::vector<TimeData> &&data);

protected:
    std::vector<FTData>     m_ftdata;
    std::vector<TimeData>   m_impulseData;
    LevelsData              m_levelsData;

    mutable std::mutex      m_dataMutex;
};

}
#endif // DATA_H
