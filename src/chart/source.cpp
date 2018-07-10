#include "source.h"

using namespace Fftchart;
Source::Source(QObject *parent) : QObject(parent)
{

}
float Source::frequency(int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _ftdata[i].frequency;
}
float Source::module(int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return 20 * log10f(_ftdata[i].data.abs() / _fftSize);
}
float Source::dataAbs(int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _ftdata[i].data.abs();
}
float Source::magnitude(int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return 20 * log10f(_ftdata[i].data.abs() / _ftdata[i].reference.abs());
}
float Source::phase(int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    constexpr const float D_PI = M_PI * 2.0;
    float p = _ftdata[i].reference.arg() - _ftdata[i].data.arg();
    while (p >  M_PI) p -= D_PI;
    while (p < -M_PI) p += D_PI;

    return p;
}
float Source::impulseTime(int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _impulseData[i].time;
}
float Source::impulseValue(int i) const noexcept
{
    if (i >= _dataLength)
        return 0.0;
    return _impulseData[i].value.real;
}
