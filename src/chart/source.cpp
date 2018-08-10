#include <algorithm>
#include "source.h"

using namespace Fftchart;
Source::Source(QObject *parent) : QObject(parent)
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

    return p;
}
float Source::impulseTime(unsigned int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _impulseData[i].time;
}
float Source::impulseValue(unsigned int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _impulseData[i].value.real;
}

void Source::copy(FTData *dataDist, TimeData *timeDist)
{
    std::copy(_ftdata, _ftdata + size(), dataDist);
    std::copy(_impulseData, _impulseData + impulseSize(), timeDist);
}
