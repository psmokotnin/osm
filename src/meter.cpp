#include "meter.h"

Meter::Meter(unsigned long size) : AudioStack(size)
{

}
void Meter::add(const float data)
{
    float d = abs(data);
    AudioStack::add(d);
    _squareValue += d;
}
void Meter::dropFirst()
{
    if (firstdata) {
        _squareValue -= firstdata->value;
    }
    AudioStack::dropFirst();
}
float Meter::value() const noexcept
{
    return 20*log10(_squareValue / _size);//dBV
}
