#include "chartable.h"

Chartable::Chartable(QObject *parent) : QIODevice(parent)
{

}
void Chartable::alloc()
{
    referenceData = (complex *)calloc(_fftSize, sizeof(complex));
    impulseData = (complex *)calloc(_fftSize, sizeof(complex));

    dataStack = new AudioStack(_fftSize);
    referenceStack = new AudioStack(_fftSize);

    data.resize(_fftSize);
}

qint64 Chartable::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);

    return -1;
}
qint64 Chartable::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return -1;
}
void Chartable::setActive(bool active)
{
    _active = active;
    emit activeChanged();
}

void Chartable::updateSeries(QAbstractSeries *series, QString type)
{
    if (!series)
        return;

    //clean
    if (type == "") {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);
        xySeries->clear();
        return;
    }

    if (type == "Scope")
        return scopeSeries(series);
    if (type == "Impulse")
        return impulseSeries(series);

        QXYSeries *xySeries = static_cast<QXYSeries *>(series);

        QVector<QPointF> points;

        int i = 0;
        float startFrequency     = 6.875, //(note A)
                frequencyFactor  = pow(2, 1.0 / _pointsPerOctave),
                currentFrequency = startFrequency,
                nextFrequency    = currentFrequency * frequencyFactor,
                currentLevel     = 0.0,
                l, y, f
                ;
        int     currentCount     = 0;

        for (i = 0; i < data.size(); i ++) {

            if (type == "RTA")
                l = data[i].module;
            else if (type == "Magnitude")
                l = data[i].magnitude;
            else if (type == "Phase")
                l = data[i].phase * 180 / M_PI;

            f = data[i].frequency;
            currentCount ++;

            if (_pointsPerOctave >= 1) {

                if (f > currentFrequency + (nextFrequency - currentFrequency) / 2) {

                    y = currentLevel / currentCount;
                    points.append(QPointF(currentFrequency, y));
                    currentLevel     = 0.0;
                    currentCount     = 0;

                    while (f > currentFrequency + (nextFrequency - currentFrequency) / 2) {
                        currentFrequency = nextFrequency;
                        nextFrequency    = currentFrequency * frequencyFactor;
                    }
                }

                currentLevel += l;
            } else {
                //without grouping by freq data
                if (f == 0)
                    f = std::numeric_limits<float>::min();
                points.append(QPointF(f, l));
            }
        }

        xySeries->replace(points);

}
void Chartable::scopeSeries(QAbstractSeries *series)
{
    QXYSeries *xySeries = static_cast<QXYSeries *>(series);

    QVector<QPointF> points;
    float trigLevel = 0.0, lastLevel;
    float x, y;
    int i, trigPoint = _fftSize / 2;
    dataStack->reset();

    for (i = 0; i < 3 * _fftSize / 4; i++, dataStack->next()) {

        if (i < _fftSize / 4) {
            lastLevel = dataStack->current();
            continue;
        }

        if (lastLevel <= trigLevel && dataStack->current() >= trigLevel) {
            trigPoint = i;
            break;
        }
        lastLevel = dataStack->current();
    }

    dataStack->reset();
    i = 0;
    do {
        x = (i - trigPoint) / 48.0;
        y = dataStack->current();
        points.append(QPointF(x, y));
        i++;
    }
    while (dataStack->next());
    xySeries->replace(points);
}
void Chartable::impulseSeries(QAbstractSeries *series)
{
    QXYSeries *xySeries = static_cast<QXYSeries *>(series);

    QVector<QPointF> points;
    float x, y;

    for (int i = 0; i < _fftSize; i ++) {
        x = i / 48.0;
        y = impulseData[i].real();
        points.append(QPointF(x, y));
    }

    xySeries->replace(points);
}
void Chartable::copyData(AudioStack *toDataStack,
              AudioStack *toReferenceStack,
              std::vector<TransferData> toData, complex *toImpulse)
{
    dataStack->reset();
    referenceStack->reset();
    for (int i = 0; i < fftSize(); i++) {
        toDataStack->add(dataStack->current());
        toReferenceStack->add(referenceStack->current());

        toData[i]    = data[i];
        toImpulse[i] = impulseData[i];

        dataStack->next();
        referenceStack->next();
    }
}
