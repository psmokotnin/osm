#include "fft.h"

FFT::FFT(QObject *parent) : QObject(parent)
{

}
QList<fftData> FFT::transform(QList<fftData> data)
{
    int n = data.length();

    for (int i = 1, j = 0; i < n; ++i)
    {
        int bit = n >> 1;
        for (; j>= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j)
            swap (data[i], data[j]);
    }

    for (int len = 2; len <= n; len <<= 1)
    {
        double ang = 2 * M_PI / len;
        fftData wlen (cos(ang), sin(ang));
        for (int i = 0; i < n; i += len)
        {
            fftData w (1);
            for (int j = 0; j < len/2; ++j)
            {
                fftData u = data[i + j],
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
