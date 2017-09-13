#ifndef FFT_H
#define FFT_H

#include <complex>
#include <QObject>
#include <fftw3.h>
#include "complex.h"

typedef std::complex<qreal> complex;
QDebug operator<<(QDebug dbg, const complex &c);
QDebug operator<<(QDebug dbg, const fftw_complex &c);
qreal fft_abs(fftw_complex c);
qreal fft_arg(fftw_complex c);
void fft_divide(fftw_complex result, fftw_complex dividend, fftw_complex divider);

class FFT : public QObject
{
    Q_OBJECT
public:
    explicit FFT(QObject *parent = nullptr);

    complex *transform(complex *data, int length, bool inverse = false);

signals:

public slots:
};

#endif // FFT_H
