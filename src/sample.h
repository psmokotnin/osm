#ifndef SAMPLE_H
#define SAMPLE_H
#include <qglobal.h>

union Sample {
    float f;
    char c[sizeof(float)];
};

#endif // SAMPLE_H
