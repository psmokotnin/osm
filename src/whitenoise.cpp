#include "whitenoise.h"

WhiteNoise::WhiteNoise(QObject *parent) : OutputDevice(parent)
{
    name = "White";
    qsrand(1);
}
Sample WhiteNoise::sample(void)
{
    Sample s;
    s.f = static_cast<float>(qrand() / RAND_MAX);
    return s;
}
