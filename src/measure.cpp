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
    delayStack = new AudioStack(delay());

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

    if (delayStack->size() != _delay) {
        delayStack->setSize(_delay);
        delayStack->fill(0.0);
    }

    for (qint64 i = 0; i < len; i += 4) {
        s.c[0] = data[i];
        s.c[1] = data[i + 1];
        s.c[2] = data[i + 2];
        s.c[3] = data[i + 3];

        if (currentChanel == _dataChanel) {
            dataStack->add((_polarity ? -1 * s.f : s.f));
        }

        if (currentChanel == _referenceChanel) {
            if (delay() > 0) {
                delayStack->add(s.f);
                referenceStack->add(delayStack->first());
            } else {
                referenceStack->add(s.f);
            }
        }

        currentChanel ++;
        if (currentChanel == _chanelCount)
            currentChanel = 0;
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
    for (int i = 0; i < _fftSize; i++) {

        workingData[i] = dataStack->current();
        workingReferenceData[i] = referenceStack->current();

        if (dataStack->current() > _level)
            _level = dataStack->current();

        if (referenceStack->current() > _referenceLevel)
            _referenceLevel = referenceStack->current();

        dataStack->next();
        referenceStack->next();
    }

    fft->transform(workingData, _fftSize);
    fft->transform(workingReferenceData, _fftSize);

    for (int i = 0; i < _fftSize; i ++) {
        workingImpulseData[i] = workingData[i] / workingReferenceData[i];
        if (i < _fftSize / 2) {
            module[i]    = 20.0 * log10(std::abs(workingData[i]));
            magnitude[i] = 20.0 * log10(std::abs(workingData[i]) / std::abs(workingReferenceData[i]));
            phase[i]     = std::arg(workingData[i]) - std::arg(workingReferenceData[i]);
            while (std::abs(phase[i]) > M_PI)
                phase[i] -= 2 * (phase[i] / std::abs(phase[i])) * M_PI;
        }
    }
    fft->transform(workingImpulseData, _fftSize, true);

    if (_setAverage > 1)
        averaging();

    memcpy(data, workingData, _fftSize *sizeof(complex));
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
        averageModule[_avgcounter][i]      = module[i];
        averageMagnitude[_avgcounter][i]   = magnitude[i];
        averagePhase[_avgcounter][i]       = phase[i];
        averageImpulseData[_avgcounter][i] = workingImpulseData[i];

        module[i] = magnitude[i] = phase[i] = 0.0;
        workingImpulseData[i] = 0.0;
        for (int j = 0; j < _average; j++) {
            module[i]       += averageModule[j][i];
            magnitude[i]    += averageMagnitude[j][i];
            phase[i]        += averagePhase[j][i];
            workingImpulseData[i] += averageImpulseData[j][i];
        }

        module[i]       /= _average;
        magnitude[i]    /= _average;
        phase[i]        /= _average;
        workingImpulseData[i] /= _average;
    }
}
void Measure::medianAveraging()
{
    _avgcounter ++;
    if (_avgcounter >= _average) _avgcounter = 0;

    for (int i = 0; i < fftSize(); i++) {
        averageModule[_avgcounter][i]      = module[i];
        averageMagnitude[_avgcounter][i]   = magnitude[i];
        averagePhase[_avgcounter][i]       = phase[i];
        averageImpulseData[_avgcounter][i] = workingImpulseData[i];

        qreal mmodule[_average], mmagnitude[_average], mphase[_average];

        module[i] = magnitude[i] = phase[i] = 0.0;
        workingImpulseData[i] = 0.0;
        for (int j = 0; j < _average; j++) {
            mmodule[j]    = averageModule[j][i];
            mmagnitude[j] = averageMagnitude[j][i];
            mphase[j]     = averagePhase[j][i];
            workingImpulseData[i] += averageImpulseData[j][i];
        }
        module[i]       = median(mmodule, _average);
        magnitude[i]    = median(mmagnitude, _average);
        phase[i]        = median(mphase, _average);
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
