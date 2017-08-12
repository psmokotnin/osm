#ifndef FFT_H
#define FFT_H

#include <complex>
#include <QObject>

#include "complex.h"

typedef std::complex<qreal> complex;

class FFT : public QObject
{
    Q_OBJECT
public:
    explicit FFT(QObject *parent = nullptr);

    complex *transform(complex *data, int length);

signals:

public slots:
};

#endif // FFT_H
