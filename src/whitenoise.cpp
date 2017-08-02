#include "whitenoise.h"

WhiteNoise::WhiteNoise(QObject *parent) : OutputDevice(parent)
{
    name = "White";
    qsrand(1);
}
Sample WhiteNoise::sample(void)
{
    Sample s;
    s.f = (float)qrand() / (float)RAND_MAX;
    return s;
}
