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
    _ftdata(nullptr),
    _impulseData(nullptr),
    _scopeData(nullptr),
    _dataLength(1),
    m_deconvolutionSize(1),
    _fftSize(1),
    _active(true)
{

}
void Source::setGlobalColor(int globalValue)
{
    if (globalValue < 19) {
        _color = Qt::GlobalColor(globalValue);
        emit colorChanged();
    }
}
float Source::frequency(unsigned int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _ftdata[i].frequency;
}
float Source::module(unsigned int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return 20 * log10f(_ftdata[i].data.abs() / _fftSize);
}
float Source::dataAbs(unsigned int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _ftdata[i].data.abs();
}
float Source::magnitude(unsigned int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return 20 * log10f(_ftdata[i].data.abs() / _ftdata[i].reference.abs());
}
float Source::phase(unsigned int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    constexpr const float
            F_PI = static_cast<float>(M_PI),
            D_PI = F_PI * 2;
    float p = _ftdata[i].reference.arg() - _ftdata[i].data.arg();
    while (p >  F_PI) p -= D_PI;
    while (p < -F_PI) p += D_PI;

    return _ftdata[i].phase;//p;
}
float Source::impulseTime(unsigned int i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return 0.0;
    return _impulseData[i].time;
}
float Source::impulseValue(unsigned int i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return 0.0;
    return _impulseData[i].value.real;
}

void Source::copy(FTData *dataDist, TimeData *timeDist)
{
    std::copy_n(_ftdata, size(), dataDist);
    std::copy_n(_impulseData, impulseSize(), timeDist);
}
