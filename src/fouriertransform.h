#ifndef FOURIERTRANSFORM_H
#define FOURIERTRANSFORM_H

#include "complex.h"

class FourierTransform
{
private:
    long _size;
    long _pointer = 0;

    //sources
    float *inA, *inB;

    //results
    complex *outputA, *outputB;

    int _octaveCount, _pointPerOctave;
    long *_lowKs;

    double kdx;

public:
    FourierTransform(int size);

    void add(float sampleA, float sampleB);

    //add and delta transform
    void change(float sampleA, float sampleB);

    void prepareDelta(int octaveCount, int pointPerOctave);
    long getPoint(int number, int octave) const;
    long getPoint(int number) const;

    long pointer() const {return _pointer;}

    complex a(int i) const;
    complex b(int i) const;

};

#endif // FOURIERTRANSFORM_H
