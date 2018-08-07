#include "sinnoise.h"

SinNoise::SinNoise(QObject *parent) : OutputDevice(parent)
{
    name = "Sin";
    connect(parent, SIGNAL(frequencyChanged(int)), this, SLOT(setFrequency(int)));
}

Sample SinNoise::sample(void)
{
    Sample output;
    sinPhase += (2.0 * M_PI * static_cast<double>(frequency) / sampleRate);
    if (sinPhase >= 2.0 * M_PI)
        sinPhase -= 2.0 * M_PI;

    output.f = static_cast<float>(sin(sinPhase));
    return output;
}
void SinNoise::setFrequency(int f)
{
    frequency = static_cast<float>(f);
}
