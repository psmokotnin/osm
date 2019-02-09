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
#include "container/array.h"

class FourierTransform
{
private:
    unsigned int _size;
    unsigned int _pointer;

    //sources
    container::array<float> inA, inB;

    //fft swap map
    container::array<unsigned int> _swapMap;

    //fast
    container::array<complex> _fastA, _fastB, wlen;

public:
    FourierTransform(unsigned int size = 2);

    void setSize(unsigned int size);

    void add(float sampleA, float sampleB);
    void set(unsigned int i, const complex &a, const complex &b);
    //ultrafast - speed up the FFT, but result can't be used for reverse fft
    void fast(WindowFunction *window, bool reverse = false, bool ultrafast = false);
    void ufast(WindowFunction *window) {
        fast(window, false, true);
    }

    void prepareFast();

    long f2i(double frequency, int sampleRate) const;

    unsigned long pointer() const {return _pointer;}

    //get fast transform results
    complex af(unsigned int i) const;
    complex bf(unsigned int i) const;
};

#endif // FOURIERTRANSFORM_H
