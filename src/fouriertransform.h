#ifndef FOURIERTRANSFORM_H
#define FOURIERTRANSFORM_H

#include "complex.h"
#include "windowfunction.h"

class FourierTransform
{
private:
    long _size;
    long _pointer = 0;

    //sources
    float *inA, *inB;

    //fast
    complex *_fastA, *_fastB, *wlen;

    //delta results
    complex *outputA, *outputB;

    int _octaveCount, _pointPerOctave;
    long *_lowKs;

    double kdx;

public:
    FourierTransform(int size);

    void add(float sampleA, float sampleB);
    void fast(WindowFunction *window);

    //add and delta transform
    void change(float sampleA, float sampleB);

    void prepareDelta(int octaveCount, int pointPerOctave);
    void prepareFast();

    long getPoint(int number, int octave) const;
    long getPoint(int number) const;
    long f2i(double frequency, int sampleRate) const;

    long pointer() const {return _pointer;}

    //get delta transform results
    complex a(int i) const;
    complex b(int i) const;

    //get fast transform results
    complex af(long i, WindowFunction *window) const;
    complex bf(long i, WindowFunction *window) const;

};

#endif // FOURIERTRANSFORM_H
