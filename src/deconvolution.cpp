#include "deconvolution.h"
#include <complex>
Deconvolution::Deconvolution(unsigned int size)
{
    _size = size;
    _pointer = 0;

    _in  = new float[_size];
    _out = new float[_size];
    _d   = new float[_size];

    _inc  = new complex[_size];
    _outc = new complex[_size];
    _dc   = new complex[_size];
    for (unsigned int i = 0; i < _size; i++) {
        _in[i] = _out[i] = 0.0;
    }

    wlen = new complex[static_cast<unsigned int>(std::log2(_size))];
    float ang, dPi = 2 * static_cast<float>(M_PI);
    for (unsigned int len = 2, l = 0; len <= _size; len <<= 1, l++) {
        ang = dPi / len;
        wlen[l] = complex(cosf(ang), sinf(ang));
    }
}
Deconvolution::~Deconvolution()
{
    delete(_in);
    delete(_out);
    delete(_d);

    delete(_inc);
    delete(_outc);
    delete(_dc);
}
void Deconvolution::add(float in, float out)
{
    _pointer++;
    if (_pointer == _size)
        _pointer = 0;

    _in[_pointer]  = in;
    _out[_pointer] = out;
}
void Deconvolution::transform()
{
    for (unsigned int i = 0; i< _size; i++) {
        _inc[i]  = _in[i];
        _outc[i] = _out[i];
    }

    //direct
    for (unsigned int i = 1, j = 0; i < _size; ++i) {
        unsigned int bit = _size >> 1;
        for (; j>= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j) {
            std::swap (_inc[i], _inc[j]);
            std::swap (_outc[i], _outc[j]);
        }
    }

    complex w, iu, iv, ou, ov;
    for (unsigned int len = 2, l = 0; len <= _size; len <<= 1, l++) {
        for (unsigned int i = 0; i < _size; i += len) {
            w = 1.0;
            for (unsigned int j = 0; j < len / 2; ++j) {
                iu                     = _inc[i + j];
                iv                     = _inc[i + j + len / 2] * w;
                _inc[i + j]            = iu + iv;
                _inc[i + j + len / 2]  = iu - iv;

                ou                     = _outc[i + j];
                ov                     = _outc[i + j + len / 2] * w;
                _outc[i + j]           = ou + ov;
                _outc[i + j + len / 2] = ou - ov;

                w *= wlen[l];
            }
        }
    }

    //devision
    for (unsigned int i = 0; i < _size; i++) {
        _dc[i] =  _outc[i] / _inc[i];
    }

    //reverse
    for (unsigned int i = 1, j = 0; i < _size; ++i) {
        unsigned int bit = _size >> 1;
        for (; j>= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j) {
            std::swap (_dc[i], _dc[j]);
        }
    }

    complex u, v;
    for (unsigned int len = 2, l = 0; len <= _size; len <<= 1, l++) {
        for (unsigned int i = 0; i < _size; i += len) {
            w = 1;
            for (unsigned int j = 0; j < len / 2; ++j) {
                u                    = _dc[i + j];
                v                    = _dc[i + j + len / 2] * w;
                _dc[i + j]           = u + v;
                _dc[i + j + len / 2] = u - v;

                w *= wlen[l].conjugate();
            }
        }
    }

    for (unsigned int i = 0; i < _size; i++) {
        _d[i] = _dc[i].real / _size;
    }
}
