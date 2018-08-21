#ifndef METER_H
#define METER_H
#include "audiostack.h"

class Meter : public AudioStack
{
    float _squareValue = 0;

public:
    Meter(unsigned long size);

    virtual void add(const float data);
    virtual void dropFirst();

    float value() const noexcept;
};

#endif // METER_H
