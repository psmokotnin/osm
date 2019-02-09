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
#include "fouriertransform.h"

#ifndef USE_SSE2
#define USE_SSE2
#endif

#include "ssemath.h"

FourierTransform::FourierTransform(unsigned int size):
    _size(size),
    _pointer(0),
    inA(_size, 0.f),
    inB(_size, 0.f)
{

}
void FourierTransform::setSize(unsigned int size)
{
    if (_size != size) {
        _size = size;
        inA.resize(_size, 0.f);
        inB.resize(_size, 0.f);
    }
}
void FourierTransform::prepareFast()
{
    _fastA.resize(_size);
    _fastB.resize(_size);
    _swapMap.resize(_size);

    auto power = static_cast<unsigned int>(log2(_size));
    wlen.resize(power);
    float ang, dPi = 2 * static_cast<float>(M_PI);
    for (unsigned int len = 2, l = 0; len <= _size; len <<= 1, l++) {
        ang = dPi / len;
        wlen[l] = complex(cosf(ang), sinf(ang));
    }

    for (unsigned int i = 0; i < _size; i++) {
        _swapMap[i] = i;
    }

    for (unsigned int i = 1, j = 0; i < _size; ++i) {
        unsigned int bit = _size >> 1;
        for (; j>= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j) {
            std::swap (_swapMap[i], _swapMap[j]);
        }
    }
}
complex FourierTransform::af(unsigned int i) const
{
    return _fastA[i];
}
complex FourierTransform::bf(unsigned int i) const
{
    return _fastB[i];
}
long FourierTransform::f2i(double frequency, int sampleRate) const
{
    return static_cast<long>(frequency * _size / sampleRate);
}
void FourierTransform::add(float sampleA, float sampleB)
{
    _pointer++;
    if (_pointer >= _size)
        _pointer = 0;

    inA[_pointer] = sampleA;
    inB[_pointer] = sampleB;
}
void FourierTransform::set(unsigned int i, const complex &a, const complex &b)
{
    _fastA[_swapMap[i]] = a;
    _fastB[_swapMap[i]] = b;
}
__attribute__((force_align_arg_pointer)) void FourierTransform::fast(WindowFunction *window, bool reverse, bool ultrafast)
{
    if (!reverse) {
        //apply data-window
        for (unsigned int i = 0, n = _pointer + 1; i < _size; i++, n++) {
            if (n >= _size) n = 0;
            _fastA[_swapMap[i]] = inA[n] * window->get(i);
            _fastB[_swapMap[i]] = inB[n] * window->get(i);
        }
    }


    complex w;//, ua, va, ub,vb;
    v4sf vw1, vw2, vu, vv, vr, v1, v2, vwl;
    unsigned long ultraLimit = _size / 2;
    bool breakloop = false;
    for (unsigned int len = 2, l = 0; len <= _size; len <<= 1, l++, breakloop = false) {

        vwl[0] = vwl[3] = wlen[l].real;
        vwl[1] = vwl[2] = reverse ? -1 * wlen[l].imag : wlen[l].imag;

        for (unsigned int i = 0, t1 = 0, t2 = len / 2; i < _size && !breakloop; i += len, t1 = i, t2 = i + len / 2) {
            w = 1.0;
            for (unsigned long j = 0; j < len / 2; ++j, ++t1, ++t2) {
                //t1 = i + j
                //t2 = i + j + len / 2

                //TODO: investigate depth skips
                if (ultrafast && len == _size && t1 > ultraLimit) {
                    breakloop = true;
                    break;
                }
                //va = _fastA[i + j + len / 2] * w;
                //vb = _fastB[i + j + len / 2] * w;
                v1[0] = _fastA[t2].real;
                v1[1] = _fastA[t2].real;
                v1[2] = _fastB[t2].real;
                v1[3] = _fastB[t2].real;

                vw1[0] = w.real;
                vw1[1] = w.imag;
                vw1[2] = w.real;
                vw1[3] = w.imag;

                v2[0] = -1.f * _fastA[t2].imag;
                v2[1] = _fastA[t2].imag;
                v2[2] = -1.f * _fastB[t2].imag;
                v2[3] = _fastB[t2].imag;

                vw2[0] = w.imag;
                vw2[1] = w.real;
                vw2[2] = w.imag;
                vw2[3] = w.real;

                v1 = _mm_mul_ps(v1, vw1);
                v2 = _mm_mul_ps(v2, vw2);
                vv = _mm_add_ps(v1, v2);

                //ua = _fastA[i + j];
                //ub = _fastB[i + j];
                vu[0] = _fastA[t1].real;//ua
                vu[1] = _fastA[t1].imag;//ua
                vu[2] = _fastB[t1].real;//ub
                vu[3] = _fastB[t1].imag;//ub

                //_fastA[i + j] = ua + va;
                //_fastB[i + j] = ub + vb;
                vr = _mm_add_ps(vu, vv);
                _fastA[t1].real = vr[0];
                _fastA[t1].imag = vr[1];
                _fastB[t1].real = vr[2];
                _fastB[t1].imag = vr[3];

                //_fastA[i + j + len / 2] = ua - va;
                //_fastB[i + j + len / 2] = ub - vb;
                vr = _mm_sub_ps(vu, vv);
                _fastA[t2].real = vr[0];
                _fastA[t2].imag = vr[1];
                _fastB[t2].real = vr[2];
                _fastB[t2].imag = vr[3];

                vw1[0] = w.real;
                vw1[1] = w.imag;
                vw1[2] = w.real;
                vw1[3] = w.imag;

                //w *= wlen[l];
                vw1 = _mm_mul_ps(vw1, vwl);
                w.real = vw1[0] - vw1[1];
                w.imag = vw1[2] + vw1[3];
            }
        }
    }
}
