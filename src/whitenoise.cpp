#include "whitenoise.h"

WhiteNoise::WhiteNoise(QObject *parent) : OutputDevice(parent)
{
    name = "White";
    qsrand(1);
}
OutputDevice::Sample WhiteNoise::sample(void)
{
    OutputDevice::Sample s;
    s.f = (float)qrand() / (float)RAND_MAX;
    return s;
}
