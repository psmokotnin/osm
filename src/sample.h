#ifndef SAMPLE_H
#define SAMPLE_H

union Sample {
    float f;
    char c[sizeof(float)];
};

#endif // SAMPLE_H
