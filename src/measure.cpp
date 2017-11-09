#include "measure.h"
/**
 * simple fft:  2^12 max
 * fftw:        2^16 max
 * deltaFT:     2^(any N as you want :-)
 */
Measure::Measure(QObject *parent) : Chartable(parent)
{
    fftPower = 15;//16 - 65K 0.73Hz;
    _fftSize = pow(2, fftPower);
    _deconvolutionSize = pow (2, 12);

    dataFT = new FourierTransform(fftSize());
    dataLength = 480;
    dataFT->prepareDelta(10, 48); //24 point per each of 10 octaves
    deconv = new Deconvolution(_deconvolutionSize);

    setAverage(1);
    alloc();

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
    audio->setBufferSize(65536*2);
    audio->start(this);

    for (int i = 0; i < dataLength; i++) {
        data[i].frequency     = (float)dataFT->getPoint(i) * sampleRate() / (float)_fftSize;
    }
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(transform()));
    timer->start(80); //12.5 per sec
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

    if (active && (
                audio->state() == QAudio::IdleState ||
                audio->state() == QAudio::StoppedState)
       )
        audio->start(this);

    if (!active && audio->state() == QAudio::ActiveState)
        audio->stop();

    _level  = 0;
    _referenceLevel = 0;
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measure::setDelay(int delay)
{
    int delta = _delay - delay;
    _delay = delay;
    referenceStack->setSize(fftSize() + _delay);
    referenceStack->rewind(delta);
}
void Measure::setAverage(int average)
{
    _setAverage = average;
}
void Measure::averageRealloc()
{
    if (_average == _setAverage)
        return;


//release old memory!
    averageData      = new complex*[_setAverage];
    averageReference = new complex*[_setAverage];
    averageMagnitude = new float*[_setAverage];
    averageDeconvolution = new float*[_setAverage];

    for (int i = 0; i < _setAverage; i ++) {
        averageData[i]      = new complex[dataLength];
        averageReference[i] = new complex[dataLength];
        averageMagnitude[i] = new float[dataLength];
        averageDeconvolution[i] = new float[_deconvolutionSize];
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
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes  = format.channelCount() * channelBytes;
    const int numSamples   = len / sampleBytes;

    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    const float *d;

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < format.channelCount(); ++j) {

            d = reinterpret_cast<const float*>(ptr);
            if (j == _dataChanel) {
                dataStack->add((_polarity ? -1.0 * *d : *d));
            }

            if (j == _referenceChanel) {
                referenceStack->add(*d);
            }

            ptr += channelBytes;
        }
    }
    return len;
}
void Measure::transform()
{
    if (!_active)
        return;

    _level = _referenceLevel = 0.0;

    while (dataStack->isNext() && referenceStack->isNext()) {
        dataStack->next();
        referenceStack->next();

        dataFT->change(dataStack->current(), referenceStack->current());
        deconv->add(referenceStack->current(), dataStack->current());
    }
    deconv->transform();
    averaging();

    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
/**
 * @brief Measure::averaging
 * vector averaging
 * TODO: add polar averaging mode
 */
void Measure::averaging()
{
    averageRealloc();

    _avgcounter ++;
    if (_avgcounter >= _average) _avgcounter = 0;

    bool overThreshold = false;
    const float threshold = _fftSize * pow(10, -90.0 / 20);

    for (int i = 0; i < dataLength ; i++) {

        averageData[_avgcounter][i]      = dataFT->a(i);
        averageReference[_avgcounter][i] = dataFT->b(i);
        overThreshold = (averageData[_avgcounter][i].abs() > threshold) &&
                (averageReference[_avgcounter][i].abs() > threshold);
        if (overThreshold)
            averageMagnitude[_avgcounter][i] = 20.0 * log10f(
                    averageData[_avgcounter][i].abs() / averageReference[_avgcounter][i].abs()
                    );

        data[i].data      = 0.0;
        data[i].reference = 0.0;
        if (overThreshold) data[i].magnitude = 0.0;

        for (int j = 0; j < _average; j++) {
            data[i].data      += averageData[j][i];
            data[i].reference += averageReference[j][i];
            if (overThreshold)
                data[i].magnitude += averageMagnitude[j][i];
        }

        data[i].module        = 20.0 * log10f(data[i].data.abs() / (_fftSize * _average));
        data[i].correct       = overThreshold;
        if (overThreshold) {
            //data[i].magnitude     = 20.0 * log10f(data[i].data.abs() / data[i].reference.abs());
            data[i].magnitude    /= _average;
            data[i].phase         = data[i].data.arg() - data[i].reference.arg();
        }

    }

    for (int i = 0; i < _deconvolutionSize; i++) {
        averageDeconvolution[_avgcounter][i]  = deconv->get(i);
        impulseData[i] = 0.0;
        for (int j = 0; j < _average; j++) {
            impulseData[i] += averageDeconvolution[j][i];
        }
        impulseData[i] /= (float)_average;
    }
}
QObject *Measure::store()
{
    Stored *store = new Stored(this);
    store->setSampleRate(sampleRate());
    store->build(this);

    return store;
}
