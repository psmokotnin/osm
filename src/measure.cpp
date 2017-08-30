#include "measure.h"

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

    for (int i = 0; i < _average; i++) {
        delete(averageData[i]);
        delete(averageReferenceData[i]);
        delete(averageImpulseData[i]);
    }

    delete(averageData);
    delete(averageReferenceData);
    delete(averageImpulseData);

    delete(workingData);
    delete(workingReferenceData);
    delete(workingImpulseData);
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
    delayStack->setSize(delay);
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

    averageData          = new complex *[_setAverage];
    averageReferenceData = new complex *[_setAverage];
    averageImpulseData   = new complex *[_setAverage];
    for (int i = 0; i < _setAverage; i ++) {
        averageData[i]          = new complex[_fftSize];
        averageReferenceData[i] = new complex[_fftSize];
        averageImpulseData[i]   = new complex[_fftSize];
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

    _avgcounter ++;
    if (_avgcounter >= _average) _avgcounter = 0;

    for (int i = 0; i < fftSize(); i++) {

        averageData[_avgcounter][i]          = workingData[i];
        averageReferenceData[_avgcounter][i] = workingReferenceData[i];
        averageImpulseData[_avgcounter][i]   = workingImpulseData[i];

        workingData[i] = workingReferenceData[i] = workingImpulseData[0] = 0;

        for (int j = 0; j < _average; j++) {
            workingData[i]          += averageData[j][i];
            workingReferenceData[i] += averageReferenceData[j][i];
            workingImpulseData[i]   += averageImpulseData[j][i];
        }
        workingData[i]          /= _average;
        workingReferenceData[i] /= _average;
        workingImpulseData[i]   /= _average;
    }
}

QObject *Measure::store()
{
    Stored *store = new Stored(this);
    store->setSampleRate(sampleRate());
    store->build(this);

    return store;
}
