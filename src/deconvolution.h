#ifndef DECONVOLUTION_H
#define DECONVOLUTION_H
#include <complex>
#include "complex.h"

class Deconvolution
{
private:
    int _size, _pointer;
    float *_in, *_out, *_d;
    complex *_inc, *_outc, *_dc;

    complex *wlen;

public:
    Deconvolution(int size);
    ~Deconvolution();
    void add(float in, float out);
    void transform();
    float get(const int i) const {return _d[i];}
};

#endif // DECONVOLUTION_H
