#include "measure.h"
#include "math.h"

Measure::Measure(QObject *parent) : Chartable(parent)
{
    fftPower = 12;
    _fftSize = pow(2, fftPower);

    workingData = (complex *)calloc(_fftSize, sizeof(complex));
    workingReferenceData = (complex *)calloc(_fftSize, sizeof(complex));
    workingImpulseData = (complex *)calloc(_fftSize, sizeof(complex));
    setAverage(1);
    alloc();

    subDataStack = new AudioStack(_fftSize);
    subReferenceStack = new AudioStack(_fftSize);

    dataStack->setSubStack(subDataStack);
    subDataStack->setParts(10);
    referenceStack->setSubStack(subReferenceStack);
    subReferenceStack->setParts(10);
    subWorkingData = (complex *)calloc(_fftSize, sizeof(complex));
    subWorkingReferenceData = (complex *)calloc(_fftSize, sizeof(complex));

    QAudioDeviceInfo d = QAudioDeviceInfo::defaultInputDevice();
    foreach (int c, d.supportedChannelCounts()) {
        if (c > _chanelCount)
            _chanelCount = c;
    }

    format.setSampleRate(48000);
    format.setChannelCount(_chanelCount);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    audio = new QAudioInput(format, this);
    open(WriteOnly);
    audio->start(this);

    fft = new FFT(this);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(transform()));
    timer->start(80); //25 per sec
}
Measure::~Measure()
{
    if (timer->isActive())
        timer->stop();

    audio->stop();
}
void Measure::setDoubleTF(bool doubleTF)
{
    if (_doubleTF != doubleTF) {
        _doubleTF = doubleTF;
    }
}
void Measure::setActive(bool active)
{
    Chartable::setActive(active);

    _level  = 0;
    _referenceLevel = 0;
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measure::setDelay(int delay)
{
    _delay = delay;
    referenceStack->setSize(fftSize() + _delay);
    subReferenceStack->setSize(fftSize() + _delay / 10);
}
void Measure::setAverage(int average)
{
    _setAverage = average;
}
void Measure::averageRealloc()
{
    if (_average == _setAverage)
        return;

    averageModule       = new qreal *[_setAverage];
    averageMagnitude    = new qreal *[_setAverage];
    averagePhase        = new qreal *[_setAverage];
    averageImpulseData  = new complex *[_setAverage];

    for (int i = 0; i < _setAverage; i ++) {
        averageModule[i]      = new qreal[_fftSize];
        averageMagnitude[i]   = new qreal[_fftSize];
        averagePhase[i]       = new qreal[_fftSize];
        averageImpulseData[i] = new complex[_fftSize];
    }

    //aply new value
    _average = _setAverage;
}
int Measure::sampleRate()
{
    return audio->format().sampleRate();
}
qint64 Measure::writeData(const char *data, qint64 len)
{
    Sample s;
    int currentChanel = 0;

    for (qint64 i = 0; i < len; i += 4, currentChanel++) {
        if (currentChanel == _chanelCount)
            currentChanel = 0;

        s.c[0] = data[i];
        s.c[1] = data[i + 1];
        s.c[2] = data[i + 2];
        s.c[3] = data[i + 3];

        if (currentChanel == _dataChanel) {
            dataStack->add((_polarity ? -1 * s.f : s.f));
        }

        if (currentChanel == _referenceChanel) {
            referenceStack->add(s.f);
        }
    }
    return len;
}
void Measure::transform()
{
    if (!_active)
        return;

    _level = _referenceLevel = 0.0;

    dataStack->reset();
    referenceStack->reset();
    subDataStack->reset();
    subReferenceStack->reset();

    for (int i = 0; i < _fftSize; i++) {

        workingData[i] = dataStack->current();
        workingReferenceData[i] = referenceStack->current();

        subWorkingData[i] = subDataStack->current();
        subWorkingReferenceData[i] = subReferenceStack->current();

        if (workingData[i].real() > _level)
            _level = workingData[i].real();

        if (workingReferenceData[i].real() > _referenceLevel)
            _referenceLevel = workingReferenceData[i].real();

        dataStack->next();
        referenceStack->next();
        subDataStack->next();
        subReferenceStack->next();
    }

    fft->transform(workingData, _fftSize);
    fft->transform(workingReferenceData, _fftSize);
    fft->transform(subWorkingData, _fftSize);
    fft->transform(subWorkingReferenceData, _fftSize);

    data.clear();
    if (_doubleTF) {
        unsigned int p = 0, offset = fftSize() / (4 * 10);
        data.resize(
                    fftSize() / 4 + //half of subData (0 to 1200Hz)
                    (fftSize() / 2 - fftSize() / (4 * 10))
                    - 1);
        complex d, r;
        for (int i = 1; i < fftSize() / 4; i++) {
            d = subWorkingData[i];
            r = subWorkingReferenceData[i];

            data[p].frequency = (qreal)i * (qreal)sampleRate() / ((qreal)fftSize() * 10.0);
            data[p].module    = 20.0 * log10(std::abs(d));
            data[p].magnitude = 20.0 * log10(std::abs(d) / std::abs(r));
            data[p].phase     = std::arg(d) - std::arg(r);
            while (std::abs(data[p].phase) > M_PI)
                data[p].phase -= 2 * (data[p].phase / std::abs(data[p].phase)) * M_PI;

            p++;
        }
        for (int i = offset; i < fftSize() / 2; i++) {
            complex d, r;
            d = workingData[i];
            r = workingReferenceData[i];

            data[p].frequency = (qreal)i * (qreal)sampleRate() / (qreal)fftSize();
            data[p].module    = 20.0 * log10(std::abs(d));
            data[p].magnitude = 20.0 * log10(std::abs(d) / std::abs(r));
            data[p].phase     = std::arg(d) - std::arg(r);
            while (std::abs(data[p].phase) > M_PI)
                data[p].phase -= 2 * (data[p].phase / std::abs(data[p].phase)) * M_PI;

            p++;
        }
    } else
        data.resize(_fftSize / 2);

    for (int i = 0; i < _fftSize; i ++) {
        workingImpulseData[i] = workingData[i] / workingReferenceData[i];
        if (!_doubleTF && i > 0 && i < _fftSize / 2) {
            data[i].frequency = i * sampleRate() / _fftSize;
            data[i].module    = 20.0 * log10(std::abs(workingData[i]));
            data[i].magnitude = 20.0 * log10(std::abs(workingData[i]) / std::abs(workingReferenceData[i]));
            data[i].phase     = std::arg(workingData[i]) - std::arg(workingReferenceData[i]);
            while (std::abs(data[i].phase) > M_PI)
                data[i].phase -= 2 * (data[i].phase / std::abs(data[i].phase)) * M_PI;
        }
    }
    fft->transform(workingImpulseData, _fftSize, true);

    if (_setAverage > 1)
        averaging();

    memcpy(referenceData, workingReferenceData, _fftSize *sizeof(complex));
    memcpy(impulseData, workingImpulseData, _fftSize *sizeof(complex));

    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measure::averaging()
{
    averageRealloc();

    if (_averageMedian)
        return medianAveraging();

    _avgcounter ++;
    if (_avgcounter >= _average) _avgcounter = 0;

    for (int i = 0; i < fftSize(); i++) {
        averageModule[_avgcounter][i]      = data[i].module;
        averageMagnitude[_avgcounter][i]   = data[i].magnitude;
        averagePhase[_avgcounter][i]       = data[i].phase;
        averageImpulseData[_avgcounter][i] = workingImpulseData[i];

        data[i].module = data[i].magnitude = data[i].phase = 0.0;
        workingImpulseData[i] = 0.0;
        for (int j = 0; j < _average; j++) {
            data[i].module        += averageModule[j][i];
            data[i].magnitude     += averageMagnitude[j][i];
            data[i].phase         += averagePhase[j][i];
            workingImpulseData[i] += averageImpulseData[j][i];
        }

        data[i].module        /= _average;
        data[i].magnitude     /= _average;
        data[i].phase         /= _average;
        workingImpulseData[i] /= _average;
    }
}
void Measure::medianAveraging()
{
    _avgcounter ++;
    if (_avgcounter >= _average) _avgcounter = 0;

    for (int i = 0; i < fftSize(); i++) {
        averageModule[_avgcounter][i]      = data[i].module;
        averageMagnitude[_avgcounter][i]   = data[i].magnitude;
        averagePhase[_avgcounter][i]       = data[i].phase;
        averageImpulseData[_avgcounter][i] = workingImpulseData[i];

        qreal mmodule[_average], mmagnitude[_average], mphase[_average];

        data[i].module = data[i].magnitude = data[i].phase = 0.0;
        workingImpulseData[i] = 0.0;
        for (int j = 0; j < _average; j++) {
            mmodule[j]    = averageModule[j][i];
            mmagnitude[j] = averageMagnitude[j][i];
            mphase[j]     = averagePhase[j][i];
            workingImpulseData[i] += averageImpulseData[j][i];
        }
        data[i].module       = median(mmodule, _average);
        data[i].magnitude    = median(mmagnitude, _average);
        data[i].phase        = median(mphase, _average);
        workingImpulseData[i] /= _average;
    }
}

QObject *Measure::store()
{
    Stored *store = new Stored(this);
    store->setSampleRate(sampleRate());
    store->build(this);

    return store;
}
