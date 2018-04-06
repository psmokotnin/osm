#include <cmath>
#include "chartable.h"

Chartable::Chartable(QObject *parent) :
    QIODevice(parent)//,
    //Fftchart::Source(parent)
{

}
void Chartable::alloc()
{
    impulseData = new complex[_deconvolutionSize];//(complex *)calloc(_deconvolutionSize, sizeof(complex));

    dataStack = new AudioStack(_fftSize);
    referenceStack = new AudioStack(_fftSize);
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

        for (i = 0; i < _dataLength; i ++) {

            if (!data[i].correct && (type == "Magnitude" || type == "Phase"))
                continue;

            if (type == "RTA")
                l = data[i].module;
            else if (type == "Magnitude")
                l = data[i].magnitude;
            else if (type == "Phase") {
                l = data[i].phase * 180.0 / M_PI;
                if (std::abs(l) > 180.0) {
                    l -= 360 * (int)(l / 180);
                }
            }

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
    AudioStack *stack = new AudioStack(dataStack);
    stack->reset();

    for (i = 0; i < 3 * _fftSize / 4; i++, stack->next()) {

        if (i < _fftSize / 4) {
            lastLevel = stack->current();
            continue;
        }

        if (lastLevel <= trigLevel && stack->current() >= trigLevel) {
            trigPoint = i;
            break;
        }
        lastLevel = stack->current();
    }

    stack->reset();
    i = 0;
    do {
        x = (i - trigPoint) / 48.0;
        y = stack->current();
        points.append(QPointF(x, y));
        i++;
    }
    while (stack->next());
    xySeries->replace(points);
}
void Chartable::impulseSeries(QAbstractSeries *series)
{
    QXYSeries *xySeries = static_cast<QXYSeries *>(series);

    QVector<QPointF> points;
    float x, y;

    for (int i = _deconvolutionSize / -2, n = _deconvolutionSize / 2;
         i < _deconvolutionSize / 2;
         i ++, n++) {
        if (n == _deconvolutionSize)
            n -= _deconvolutionSize;

        x = (float)i * 1000.0 / sampleRate(); //i -> ms
        y = impulseData[n].real;

        points.append(QPointF(x, y));
    }

    xySeries->replace(points);
}
void Chartable::copyData(AudioStack *toDataStack,
              AudioStack *toReferenceStack,
              std::vector<TransferData> *toData, complex *toImpulse)
{
    dataStack->reset();
    referenceStack->reset();
    for (int i = 0; i < fftSize(); i++) {
        toDataStack->add(dataStack->current());
        toReferenceStack->add(referenceStack->current());

        //toImpulse[i] = impulseData[i];

        dataStack->next();
        referenceStack->next();
    }

//    toData->resize(data.size());
//    for (unsigned long i = 0; i < data.size(); i++)
//        (*toData)[i] = data[i];
}

int Chartable::dataLength(Fftchart::Type *type) const
{
    switch (*type) {
    case Fftchart::Type::RTA:
    case Fftchart::Type::Magnitude:
    case Fftchart::Type::Phase:
        return _dataLength;

    case Fftchart::Type::Impulse:
        return _deconvolutionSize;

    case Fftchart::Type::Scope:
        return dataStack->size();
    default:
        return 0;
    }
}
//BUG: crash if i out of bounds Thread 2 Crashed:: QSGRenderThread 0   com.yourcompany.OpenSoundMeter	0x0000000107f8c463 Chartable::x(Fftchart::Type, int) const + 19
//when reduce fftPower
float Chartable::x(Fftchart::Type *type, int i) const
{
    switch (*type) {
    case Fftchart::Type::RTA:
    case Fftchart::Type::Magnitude:
    case Fftchart::Type::Phase:
        return data[i].frequency;

    case Fftchart::Type::Impulse:
        return (i - _deconvolutionSize / -2) * 1000.0 / sampleRate();

    case Fftchart::Type::Scope:
        //return dataStack->size();
        break;
    default:
        return 0;
    }
}
float Chartable::y(Fftchart::Type *type, int i) const
{
    static const float phaseMul = 180.0 / M_PI;
    switch (*type) {
    case Fftchart::Type::RTA:
        return data[i].module;
    case Fftchart::Type::Magnitude:
        return data[i].magnitude;
    case Fftchart::Type::Phase:
        return data[i].phase * phaseMul;

    case Fftchart::Type::Impulse:
        if (i > _deconvolutionSize / 2)
            i-= _deconvolutionSize;
        return impulseData[i].real;

    case Fftchart::Type::Scope:
        //return dataStack->size();
        break;
    default:
        return 0;
    }
}
