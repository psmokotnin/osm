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
#ifndef DECONVOLUTION_H
#define DECONVOLUTION_H
#include <complex>
#include "complex.h"

class Deconvolution
{
private:
    unsigned int _size, _pointer, _maxPoint;
    float *_in, *_out, *_d;
    complex *_inc, *_outc, *_dc;

    complex *wlen;

public:
    Deconvolution(unsigned int size);
    ~Deconvolution();
    void add(float in, float out);
    void transform();
    float get(const unsigned int i) const {return _d[i];}
    unsigned int maxPoint() const noexcept {return _maxPoint;}
};

#endif // DECONVOLUTION_H
