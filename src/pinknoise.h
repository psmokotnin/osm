#ifndef PINKNOISE_H
#define PINKNOISE_H

#include "outputdevice.h"

class PinkNoise : public OutputDevice
{
    Q_OBJECT

public:
    PinkNoise(QObject *parent);

private:

    OutputDevice::Sample sample(void);
    unsigned long pseudoRandom(void);

    const static int MAX_RANDOM_ROWS = 30;
    const static int RANDOM_BITS     = 24;
    const static int RANDOM_SHIFT    = ((sizeof(long) * 8) - PinkNoise::RANDOM_BITS);

    long      rows[PinkNoise::MAX_RANDOM_ROWS];
    long      runningSum;   // Used to optimize summing of generators.
    int       index;        // Incremented each sample.
    int       indexMask;    // Index wrapped by ANDing with this mask.
    float     scalar;       // Used to scale within range of -1.0 to +1.0
};

#endif // PINKNOISE_H
