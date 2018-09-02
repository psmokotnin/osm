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

FourierTransform::FourierTransform(unsigned int size)
{
    setSize(size);
}
void FourierTransform::setSize(unsigned int size)
{
    if (_size != size) {
        _size = size;
        inA = new float[_size];
        inB = new float[_size];

        dA = new float[_size];
        dB = new float[_size];

        for (unsigned int i = 0; i < _size; i++) {
            inA[i] = inB[i] = dA[i] = dB[i] = 0.0;
        }
    }
}
void FourierTransform::prepareDelta(unsigned int octaveCount, unsigned int pointPerOctave)
{
    kdx = static_cast<float>(-2.0 * M_PI / _size);
    _octaveCount    = octaveCount;
    _pointPerOctave = pointPerOctave;

    _lowKs = new long[_pointPerOctave];
    double leftK = _size / (2.0 * pow(2.0, octaveCount)), rightK = 2.0 * leftK;

    double a = pow(rightK / leftK, 1.0 / _pointPerOctave);

    for (unsigned int i = 0; i < _pointPerOctave; i++) {
        _lowKs[i] = static_cast<long>(leftK * pow(a, i));
    }

    outputA = new complex[_octaveCount * _pointPerOctave];
    outputB = new complex[_octaveCount * _pointPerOctave];

    for (unsigned int i = 0; i < _octaveCount * _pointPerOctave; i++) {
        outputA[i] = outputB[i] = 0.0;
    }
}
void FourierTransform::prepareFast()
{
    _fastA = new complex[_size];
    _fastB = new complex[_size];
    _doubleA = new complex[_size];
    _doubleB = new complex[_size];
    _swapMap = new unsigned long[_size];

    unsigned int power = static_cast<unsigned int>(log2(_size));
    wlen = new complex[power];
    float ang, dPi = 2 * static_cast<float>(M_PI);
    for (unsigned int len = 2, l = 0; len <= _size; len <<= 1, l++) {
        ang = dPi / len;
        wlen[l] = complex(cosf(ang), sinf(ang));
    }

    for (unsigned long i = 0; i < _size; i++) {
        _swapMap[i] = i;
    }

    for (unsigned long i = 1, j = 0; i < _size; ++i) {
        unsigned long bit = _size >> 1;
        for (; j>= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j) {
            std::swap (_swapMap[i], _swapMap[j]);
        }
    }
}
long FourierTransform::getPoint(unsigned int number, unsigned int octave) const
{
    return _lowKs[number] * static_cast<long>(pow(2, octave));
}
long FourierTransform::getPoint(unsigned int number) const
{
    unsigned int octave = number / _pointPerOctave;
    number -= octave * _pointPerOctave;
    return getPoint(number, octave);
}
complex FourierTransform::a(int i) const
{
    return outputA[i];
}
complex FourierTransform::b(int i) const
{
    return outputB[i];
}
complex FourierTransform::af(long i) const
{
    return _fastA[i];
}
complex FourierTransform::bf(long i) const
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

__attribute__((force_align_arg_pointer)) void FourierTransform::change(float sampleA, float sampleB)
{
    _pointer++;
    if (_pointer >= _size)
        _pointer = 0;

    float dx   = kdx * _pointer;
    float
          dInA = sampleA - inA[_pointer],
          dInB = sampleB - inB[_pointer];

    v4sf x, ys, yc, ysPre, ycPre;
    unsigned int totalPoints = _pointPerOctave * _octaveCount;
    for (unsigned int i = 0; i < _pointPerOctave; i += 4) {
      for (unsigned int op = 0; op < totalPoints; op += _pointPerOctave) {

        if (op == 0) {
            x[0] = _lowKs[i    ] * dx;
            x[1] = _lowKs[i + 1] * dx;
            x[2] = _lowKs[i + 2] * dx;
            x[3] = _lowKs[i + 3] * dx;

            sincos_ps(x, &ys, &yc);//very hard to calculate, do it once and use results for each next octave.
        } else {
            ysPre = ys;
            ycPre = yc;

            //sin(2x) = 2sin(x)cos(x)
            ys = _mm_mul_ps(ysPre, ycPre);
            ys = _mm_add_ps(ys,    ys);

            //cos(2x) = cos(x)^2 - sin(x)^2
            ycPre = _mm_mul_ps(ycPre, ycPre);
            ysPre = _mm_mul_ps(ysPre, ysPre);
            yc    = _mm_sub_ps(ycPre, ysPre);
        }

        outputA[ (i    ) + op ].real += yc[0] * dInA;
        outputA[ (i + 1) + op ].real += yc[1] * dInA;
        outputA[ (i + 2) + op ].real += yc[2] * dInA;
        outputA[ (i + 3) + op ].real += yc[3] * dInA;

        outputB[ (i    ) + op ].real += yc[0] * dInB;
        outputB[ (i + 1) + op ].real += yc[1] * dInB;
        outputB[ (i + 2) + op ].real += yc[2] * dInB;
        outputB[ (i + 3) + op ].real += yc[3] * dInB;

        outputA[ (i    ) + op ].imag += ys[0] * dInA;
        outputA[ (i + 1) + op ].imag += ys[1] * dInA;
        outputA[ (i + 2) + op ].imag += ys[2] * dInA;
        outputA[ (i + 3) + op ].imag += ys[3] * dInA;

        outputB[ (i    ) + op ].imag += ys[0] * dInB;
        outputB[ (i + 1) + op ].imag += ys[1] * dInB;
        outputB[ (i + 2) + op ].imag += ys[2] * dInB;
        outputB[ (i + 3) + op ].imag += ys[3] * dInB;
      }
    }

    inA[_pointer] = sampleA;
    inB[_pointer] = sampleB;
}
void FourierTransform::fast(WindowFunction *window)
{
    //apply data-window
    for (unsigned long i = 0, n = _pointer + 1; i < _size; i++, n++) {
        if (n >= _size) n = 0;
        _fastA[_swapMap[i]] = inA[n] * window->get(i);
        _fastB[_swapMap[i]] = inB[n] * window->get(i);
    }

    complex w, u, v;
    for (unsigned long len = 2, l = 0; len <= _size; len <<= 1, l++) {
        for (unsigned long i = 0; i < _size; i += len) {
            w = 1.0;
            for (unsigned long j = 0; j < len / 2; ++j) {
                u                     = _fastA[i + j];
                v                     = _fastA[i + j + len / 2] * w;
                _fastA[i + j]            = u + v;
                _fastA[i + j + len / 2]  = u - v;

                u                     = _fastB[i + j];
                v                     = _fastB[i + j + len / 2] * w;
                _fastB[i + j]           = u + v;
                _fastB[i + j + len / 2] = u - v;
                w *= wlen[l];
            }
        }
    }
}
