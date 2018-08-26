/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FOURIERTRANSFORM_H
#define FOURIERTRANSFORM_H

#include "complex.h"
#include "windowfunction.h"

class FourierTransform
{
private:
    unsigned long _size;
    unsigned long _pointer = 0;
    const unsigned int _dataDivider = 4;

    //sources
    float *inA, *inB, *dA, *dB;

    //fft swap map
    unsigned long *_swapMap;

    //fast
    complex *_fastA, *_fastB, *wlen;
    complex *_doubleA, *_doubleB;

    //delta results
    complex *outputA, *outputB;

    unsigned int _octaveCount, _pointPerOctave;
    long *_lowKs;

    float kdx;

public:
    FourierTransform(unsigned int size);

    void setSize(unsigned int size);

    void add(float sampleA, float sampleB);
    void fast(WindowFunction *window);

    //add and delta transform
    void change(float sampleA, float sampleB);

    void prepareDelta(unsigned int octaveCount, unsigned int pointPerOctave);
    void prepareFast();

    long getPoint(unsigned int number, unsigned int octave) const;
    long getPoint(unsigned int number) const;
    long f2i(double frequency, int sampleRate) const;

    unsigned long pointer() const {return _pointer;}

    //get delta transform results
    complex a(int i) const;
    complex b(int i) const;

    //get fast transform results
    complex af(long i) const;
    complex bf(long i) const;

    unsigned int dataDivider() const { return _dataDivider;}
};

#endif // FOURIERTRANSFORM_H
