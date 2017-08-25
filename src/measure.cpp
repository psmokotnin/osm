#include "measure.h"

Measure::Measure(QObject *parent) : Chartable(parent)
{
    fftPower = 12;
    _fftSize = pow(2, fftPower);

    workingData = (complex *)calloc(_fftSize, sizeof(complex));
    workingReferenceData = (complex *)calloc(_fftSize, sizeof(complex));
    workingImpulseData = (complex *)calloc(_fftSize, sizeof(complex));

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
void Measure::setActive(bool active)
{
    Chartable::setActive(active);

    _level  = 0;
    _referenceLevel = 0;
    emit levelChanged();
}
void Measure::setDelay(int delay)
{
    delayStack->setSize(delay);
    _delay = delay;
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
            dataStack->add(s.f);
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

    memcpy(data, workingData, _fftSize *sizeof(complex));
    memcpy(referenceData, workingReferenceData, _fftSize *sizeof(complex));
    memcpy(impulseData, workingImpulseData, _fftSize *sizeof(complex));

    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
QObject *Measure::store()
{
    Stored *store = new Stored(this);
    store->setSampleRate(sampleRate());
    store->build(this);

    return store;
}
