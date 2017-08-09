#ifndef FFT_H
#define FFT_H

#include <complex>
#include <QObject>

#include "complex.h"

typedef std::complex<qreal> fftData;

class FFT : public QObject
{
    Q_OBJECT
public:
    explicit FFT(QObject *parent = nullptr);

    QList<fftData> transform(QList<fftData> data);

signals:

public slots:
};

#endif // FFT_H
