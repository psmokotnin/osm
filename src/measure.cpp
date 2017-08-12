#include <QtCharts/QXYSeries>

#include "measure.h"

Measure::Measure(QObject *parent) : QObject(parent)
{
    fftPower = 12;
    fftSize = pow(2, fftPower);

    data = (complex *)calloc(fftSize, sizeof(complex));

    fft = new FFT(this);
}
void Measure::setActive(bool active)
{
    _active = active;
    _level  = 0;

    emit activeChanged();
    emit levelChanged();
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
    if (!_active)
        return;

    foreach (Sample s, source->buffer) {
        buffer.append(s);
        while (buffer.length() > fftSize) {
            buffer.removeFirst();
        }
    }

    _level = 0.0;
    int i = 0;
    foreach (Sample s, buffer) {
        data[i] = s.f;
        i++;
        if (s.f > _level) _level = s.f;
    }

    fft->transform(data, fftSize);

    emit readyRead();
    emit levelChanged();
}


void Measure::updateRTASeries(QAbstractSeries *series)
{
    if (series) {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);

        QVector<QPointF> points;

        int i = 0;
        qreal startFrequency     = 6.875, //(note A)
                frequencyFactor  = pow(2, 1.0 / _pointsPerOctave),
                currentFrequency = startFrequency,
                nextFrequency    = currentFrequency * frequencyFactor,
                currentLevel     = 0.0,
                rateFactor       = source->sampleRate() / fftSize,
                m, y, f
                ;
        int     currentCount     = 0;

        for (i = 0; i < fftSize / 2; i ++) {
            m = std::abs(data[i]);
            y = 20.0 * log10(m); // log10(m / 1.0); 1.0f - 0dB point
            f = i * rateFactor;
            currentCount ++;

            if (_pointsPerOctave >= 1) {

                if (f > currentFrequency + (nextFrequency - currentFrequency) / 2) {

                    y = 20.0 * log10(currentLevel / currentCount);
                    points.append(QPointF(currentFrequency, y));
                    currentLevel     = 0.0;
                    currentCount     = 0;

                    while (f > currentFrequency + (nextFrequency - currentFrequency) / 2) {
                        currentFrequency = nextFrequency;
                        nextFrequency    = currentFrequency * frequencyFactor;
                    }
                }

                currentLevel += m;
            } else {
                //without grouping by freq data
                if (f == 0)
                    f = std::numeric_limits<qreal>::min();
                points.append(QPointF(f, y));
            }
        }

        xySeries->replace(points);
    }
}
