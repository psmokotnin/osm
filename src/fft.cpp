#include "fft.h"
#include <complex>

using namespace std::complex_literals;

QDebug operator<<(QDebug dbg, const complex &c)
{
    dbg.nospace() << "Complex value: r:"
        << c.real() << " i:" << c.imag();

    return dbg.maybeSpace();
}
QDebug operator<<(QDebug dbg, const fftw_complex &c)
{
    dbg.nospace() << "Complex value: r:"
        << c[0] << " i:" << c[1];

    return dbg.maybeSpace();
}
qreal fft_abs(fftw_complex c) {
    return sqrt(pow(c[0], 2) + pow(c[1], 2));
}
qreal fft_arg(fftw_complex c) {
    return atan2(c[1], c[0]);
}

void fft_divide(fftw_complex result, fftw_complex dividend, fftw_complex divider) {

    result[0] = (dividend[0] * divider[0] + dividend[1] * divider[1]) /
            (pow(divider[0], 2) + pow(divider[1], 2));

    result[1] = (dividend[1] * divider[0] - dividend[0] * divider[1]) /
            (pow(divider[0], 2) + pow(divider[1], 2));
}

FFT::FFT(QObject *parent) : QObject(parent)
{

}
complex *FFT::transform(complex *data, int length, bool inverse)
{
    int n = length;

    for (int i = 1, j = 0; i < n; ++i)
    {
        int bit = n >> 1;
        for (; j>= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j)
            swap (data[i], data[j]);
    }

    complex wlen, w, u, v;
    float ang, dPi = 2 * M_PI * (inverse ? -1 : 1);
    for (int len = 2; len <= n; len <<= 1)
    {
        ang = dPi / len;
        wlen = cos(ang) + sin(ang) * 1i;
        for (int i = 0; i < n; i += len)
        {
            w = 1;
            for (int j = 0; j < len / 2; ++j)
            {
                u = data[i + j];
                v = data[i + j + len / 2] * w;

                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    for (int i = 0; i < n; ++i)
        data[i] /= n;

    return data;
}
