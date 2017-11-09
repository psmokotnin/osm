#include "fouriertransform.h"

#define USE_SSE2
#include "ssemath.h"


FourierTransform::FourierTransform(int size)
{
    _size = size;
    kdx = -2.0 * M_PI / (double)_size;
    inA = new float[_size];
    inB = new float[_size];

    for (int i = 0; i < _size; i++) {
        inA[i] = inB[i] = 0.0;
    }
}
void FourierTransform::prepareDelta(int octaveCount, int pointPerOctave)
{
    _octaveCount    = octaveCount;
    _pointPerOctave = pointPerOctave;

    _lowKs = new long[_pointPerOctave];
    double leftK = (double)_size / (2.0 * pow(2.0, octaveCount)), rightK = 2.0 * leftK;

    double a = pow((double)rightK / leftK, 1.0 / _pointPerOctave);

    for (int i = 0; i < _pointPerOctave; i++) {
        _lowKs[i] = leftK * pow(a, i);
    }

    outputA = new complex[_octaveCount * _pointPerOctave];
    outputB = new complex[_octaveCount * _pointPerOctave];

    for (int i = 0; i < _octaveCount * _pointPerOctave; i++) {
        outputA[i] = outputB[i] = 0.0;
    }
}
long FourierTransform::getPoint(int number, int octave) const
{
    return _lowKs[number] * pow(2, octave);
}
long FourierTransform::getPoint(int number) const
{
    int octave = number / _pointPerOctave;
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
void FourierTransform::add(float sampleA, float sampleB)
{
    _pointer++;
    if (_pointer >= _size)
        _pointer = 0;

    inA[_pointer] = sampleA;
    inB[_pointer] = sampleB;
}

void FourierTransform::change(float sampleA, float sampleB)
{
    _pointer++;
    if (_pointer >= _size)
        _pointer = 0;

    double dx   = kdx * _pointer;
    float
          dInA = sampleA - inA[_pointer],
          dInB = sampleB - inB[_pointer];

    v4sf x, ys, yc, ysPre, ycPre;
    int totalPoints = _pointPerOctave * _octaveCount;
    for (int i = 0; i < _pointPerOctave; i += 4) {
      for (int op = 0; op < totalPoints; op += _pointPerOctave) {

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
