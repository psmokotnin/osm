#include "fouriertransform.h"

#define USE_SSE2
#include "ssemath.h"

FourierTransform::FourierTransform(int size)
{
    setSize(size);
}

bool FourierTransform::doubleTW() const
{
    return _doubleTW;
}

void FourierTransform::setDoubleTW(bool doubleTW)
{
    _doubleTW = doubleTW;
}

void FourierTransform::setSize(int size)
{
    if (_size != size) {
        _size = size;
        inA = new float[_size];
        inB = new float[_size];

        dA = new float[_size];
        dB = new float[_size];

        for (int i = 0; i < _size; i++) {
            inA[i] = inB[i] = dA[i] = dB[i] = 0.0;
        }
    }
}
void FourierTransform::prepareDelta(int octaveCount, int pointPerOctave)
{
    kdx = -2.0 * M_PI / (double)_size;
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
void FourierTransform::prepareFast()
{
    _fastA = new complex[_size];
    _fastB = new complex[_size];
    _doubleA = new complex[_size];
    _doubleB = new complex[_size];

    wlen = new complex[(int)log2(_size)];
    float ang, dPi = 2 * M_PI;
    for (int len = 2, l = 0; len <= _size; len <<= 1, l++) {
        ang = dPi / len;
        wlen[l] = complex(cosf(ang), sinf(ang));
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
complex FourierTransform::af(long i, WindowFunction *window) const
{
    return _fastA[i] / window->gain();
}
complex FourierTransform::bf(long i, WindowFunction *window) const
{
    return _fastB[i] / window->gain();
}
complex FourierTransform::ad(long i, WindowFunction *window) const
{
    return _doubleA[i] / window->gain();
}
complex FourierTransform::bd(long i, WindowFunction *window) const
{
    return _doubleB[i] / window->gain();
}
long FourierTransform::f2i(double frequency, int sampleRate) const
{
    return frequency * _size / sampleRate;
}
void FourierTransform::add(float sampleA, float sampleB)
{
    _pointer++;
    if (_pointer >= _size)
        _pointer = 0;

    inA[_pointer] = sampleA;
    inB[_pointer] = sampleB;

    //divide sample rate
    if (_doubleTW) {
        if (_pointer % _dataDivider == 0) {
            _doublePointer++;
            if (_doublePointer >= _size)
                _doublePointer = 0;

            dA[_doublePointer] = accA;
            dB[_doublePointer] = accB;
            accA = accB = 0.0;
        }

        accA += sampleA / (float)_dataDivider;
        accB += sampleB / (float)_dataDivider;
    }
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
void FourierTransform::fast(WindowFunction *window)
{
    //apply data-window
    for (int i = 0, n = _pointer + 1; i < _size; i++, n++) {
        if (n >= _size) n = 0;
        _fastA[i] = inA[n] * window->get(i);
        _fastB[i] = inB[n] * window->get(i);
    }

    if (_doubleTW) {
        for (int i = 0, n = _doublePointer + 1; i < _size; i++, n++) {
            if (n >= _size) n = 0;
            _doubleA[i] = dA[n] * window->get(i);
            _doubleB[i] = dB[n] * window->get(i);
        }
    }

    for (int i = 1, j = 0; i < _size; ++i) {
        int bit = _size >> 1;
        for (; j>= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j) {
            std::swap (_fastA[i], _fastA[j]);
            std::swap (_fastB[i], _fastB[j]);

            if (_doubleTW) {
                std::swap (_doubleA[i], _doubleA[j]);
                std::swap (_doubleB[i], _doubleB[j]);
            }
        }
    }

    complex w, u, v;
    for (int len = 2, l = 0; len <= _size; len <<= 1, l++) {
        for (int i = 0; i < _size; i += len) {
            w = 1.0;
            for (int j = 0; j < len / 2; ++j) {
                u                     = _fastA[i + j];
                v                     = _fastA[i + j + len / 2] * w;
                _fastA[i + j]            = u + v;
                _fastA[i + j + len / 2]  = u - v;

                u                     = _fastB[i + j];
                v                     = _fastB[i + j + len / 2] * w;
                _fastB[i + j]           = u + v;
                _fastB[i + j + len / 2] = u - v;

                if (_doubleTW) {
                    u                     = _doubleA[i + j];
                    v                     = _doubleA[i + j + len / 2] * w;
                    _doubleA[i + j]            = u + v;
                    _doubleA[i + j + len / 2]  = u - v;

                    u                     = _doubleB[i + j];
                    v                     = _doubleB[i + j + len / 2] * w;
                    _doubleB[i + j]           = u + v;
                    _doubleB[i + j + len / 2] = u - v;
                }

                w *= wlen[l];
            }
        }
    }
}
