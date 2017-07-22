#ifndef WHITENOISE_H
#define WHITENOISE_H

#include "outputdevice.h"

class WhiteNoise : public OutputDevice
{
    Q_OBJECT

public:
    WhiteNoise(QObject *parent);

private:
    OutputDevice::Sample sample(void);
};

#endif // WHITENOISE_H
