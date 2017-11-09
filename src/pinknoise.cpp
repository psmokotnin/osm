#include "pinknoise.h"

PinkNoise::PinkNoise(QObject *parent) : OutputDevice(parent)
{
    name = "Pink";

    long pmax;
    int numRows = 12;

    index = 0;
    indexMask = (1 << numRows) - 1;
    /* Calculate maximum possible signed random value. Extra 1 for white noise always added. */
    pmax = (numRows + 1) * (1 << (RANDOM_BITS - 1));
    scalar = 1.0f / pmax;

    /* Initialize rows. */
    for (int i = 0; i < numRows; i++)
        rows[i] = 0;

    runningSum = 0;
}
Sample PinkNoise::sample(void)
{
    long newRandom;
    long sum;
    Sample output;

    /* Increment and mask index. */
    index = (index + 1) & indexMask;

    /* If index is zero, don't update any random values. */
    if (index != 0 ) {
        /* Determine how many trailing zeros in PinkIndex. */
        /* This algorithm will hang if n==0 so test first. */
        int numZeros = 0;
        int n = index;
        while ((n & 1) == 0) {
            n = n >> 1;
            numZeros++;
        }

        /* Replace the indexed ROWS random value.
         * Subtract and add back to RunningSum instead of adding all the random
         * values together. Only one changes each time.
         */
        runningSum -= rows[numZeros];
        newRandom = ((long)pseudoRandom()) >> this->RANDOM_SHIFT;
        runningSum += newRandom;
        rows[numZeros] = newRandom;
    }

    /* Add extra white noise value. */
    newRandom = ((long)pseudoRandom()) >> this->RANDOM_SHIFT;
    sum = runningSum + newRandom;

    /* Scale to range of -1.0 to 0.9999. */
    output.f = scalar * sum;

    return output;
}

/* Calculate pseudo-random 32 bit number based on linear congruential method. */
unsigned long PinkNoise::pseudoRandom( void )
{
    static unsigned long randSeed = 22222;  /* Change this for different random sequences. */
    randSeed = (randSeed * 196314165) + 907633515;
    return randSeed;
}
