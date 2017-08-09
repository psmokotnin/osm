#include <QtCharts/QXYSeries>

#include "measure.h"

Measure::Measure(QObject *parent) : QObject(parent)
{
    fftPower = 11;
    fftSize = pow(2, fftPower);

    fft = new FFT(this);
}
void Measure::setSource(Source *s)
{
    source = s;
    connect(source, SIGNAL(readyRead()), SLOT(reciveData()));
}
/**
 * @brief Measure::reciveData
 * When reciveData slot called:
 * At first Measure appends data from the source to the internal buffer,
 * Then it prepares data for FFT and run
 * Finally create data for charts and emit signals
 */
void Measure::reciveData()
{
    foreach (Sample s, source->buffer) {
        buffer.append(s);
        while (buffer.length() > fftSize) {
            buffer.removeFirst();
        }
    }

    data.clear();

    foreach (Sample s, buffer) {
        data.append(s.f);
    }

    data = fft->transform(data);

    emit readyRead();
}

void Measure::updateRTASeries(QAbstractSeries *series)
{
    if (series) {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);

        QVector<QPointF> points;

        int i = 1;
        foreach (fftData d, data) {
            qreal m = sqrt(d.real() * d.real() + d.imag() * d.imag());
            qreal y = 20.0 * log10(m / 1.0);

            points.append(QPointF(i * source->sampleRate() / fftSize, y));
            if (i == fftSize / 2)
                break;
            i++;
        }

        xySeries->replace(points);
    }
}
