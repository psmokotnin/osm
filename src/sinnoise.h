#ifndef SINNOISE_H
#define SINNOISE_H

#include <QtMath>
#include "outputdevice.h"

class SinNoise : public OutputDevice
{
    Q_OBJECT

public:
    SinNoise(QObject *parent);

public slots:
    void setFrequency(int f);

private:
    Sample sample(void);

    float frequency = 1000;
    double sinPhase  = 0;
};

#endif // SINNOISE_H
