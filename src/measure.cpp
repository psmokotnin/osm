#include <algorithm>
#include "measure.h"
/**
 * simple fft:  2^12 max
 * fftw:        2^16 max
 * deltaFT:     2^(any N as you want :-)
 */
Measure::Measure(QObject *parent) : Fftchart::Source(parent)
{
    _iodevice = new InputDevice(this);
    connect(_iodevice, SIGNAL(recived(const char *, qint64)), SLOT(writeData(const char *, qint64)));

    _fftPower = 14;//16 - 65K 0.73Hz;
    _fftSize = pow(2, _fftPower);
    _deconvolutionSize = pow (2, 12);

    _dataFT  = new FourierTransform(fftSize());
    _window = new WindowFunction(fftSize());
    _window->setType(WindowFunction::Type::hann);

    _device = QAudioDeviceInfo::defaultInputDevice();
    _chanelCount = std::max(_dataChanel, _referenceChanel) + 1;
    foreach (int c, _device.supportedChannelCounts()) {
        if (c > _chanelCount)
            _chanelCount = c;
    }

    _format.setSampleRate(48000);
    _format.setChannelCount(_chanelCount);
    _format.setSampleSize(32);
    _format.setCodec("audio/pcm");
    _format.setByteOrder(QAudioFormat::LittleEndian);
    _format.setSampleType(QAudioFormat::Float);

    _audio = new QAudioInput(_device, _format, this);
    _iodevice->open(InputDevice::WriteOnly);
    _audio->setBufferSize(65536*2);

    calculateDataLength();
    _dataFT->prepareFast();

    _deconv = new Deconvolution(_deconvolutionSize);
    _impulseData = new TimeData[_deconvolutionSize];

    impulseData = new complex[_deconvolutionSize];

    dataStack = new AudioStack(_fftSize);
    referenceStack = new AudioStack(_fftSize);

    setAverage(1);

    _audio->start(_iodevice);
    _chanelCount = _format.channelCount();
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), SLOT(transform()));
    _timer->start(80); //12.5 per sec
}
Measure::~Measure()
{
    disconnect(this);
    if (_timer->isActive())
        _timer->stop();

    _audio->stop();
}
QVariant Measure::getDeviceList(void)
{
    QStringList deviceList;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        deviceList << deviceInfo.deviceName();
    }
    return QVariant::fromValue(deviceList);
}
QString Measure::deviceName()
{
    return _device.deviceName();
}
void Measure::selectDevice(QString name)
{
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (name == deviceInfo.deviceName()) {
            _audio->stop();
            _iodevice->close();

            _device = deviceInfo;

            _chanelCount = std::max(_dataChanel, _referenceChanel) + 1;
            foreach (int c, _device.supportedChannelCounts()) {
                if (c > _chanelCount)
                    _chanelCount = c;
            }

            _audio = new QAudioInput(_device, _format, this);
            _iodevice->open(InputDevice::WriteOnly);
            _audio->setBufferSize(65536 * _chanelCount);
            if (active())
                _audio->start(_iodevice);
            _chanelCount = _format.channelCount();
        }
    }
}
void Measure::setFftPower(int power)
{
    if (_fftPower != power) {
        _audio->suspend();

        _fftPower = power;
        _fftSize  = pow(2, _fftPower);

        _dataFT->setSize(_fftSize);
        _dataFT->prepareFast();
        _window->setSize(_fftSize);

        calculateDataLength();

        averageRealloc(true);

        _audio->resume();
    }
}

void Measure::setDoubleTW(bool doubleTW)
{
    _audio->suspend();

    _dataFT->setDoubleTW(doubleTW);
    calculateDataLength();
    averageRealloc(true);

    _audio->resume();
}
void Measure::calculateDataLength()
{
    if (_dataFT->doubleTW()) {
//TODO: remove
        _dataLength =
                  _fftSize / 10 +                                           //subData (0 to 1200Hz)
                 (_fftSize / 2 - _fftSize / (10 * _dataFT->dataDivider())); //from 1.2 to 24kHz

        _ftdata = new FTData[_dataLength];
        for (int i = 0; i < _dataLength; i++) {

            if (i < _fftSize / 10) {
                _ftdata[i].fftPoint  = i;
                _ftdata[i].frequency = (float)i * sampleRate() / ((float)_fftSize * _dataFT->dataDivider());
            } else {
                _ftdata[i].fftPoint  = i - _fftSize / 10 + _fftSize / (10 * _dataFT->dataDivider());
                _ftdata[i].frequency = (float)_ftdata[i].fftPoint * sampleRate() / (float)_fftSize;
            }
        }

    } else {

        //classic fft
        _dataLength = _fftSize / 2;
        _ftdata = new FTData[_dataLength];
        for (int i = 0; i < _dataLength; i++) {
            _ftdata[i].fftPoint  = i;
            _ftdata[i].frequency = (float)i * sampleRate() / (float)_fftSize;
        }
    }
}
void Measure::setActive(bool active)
{
    Fftchart::Source::setActive(active);

    if (active && (
                _audio->state() == QAudio::IdleState ||
                _audio->state() == QAudio::StoppedState)
       )
        _audio->start(_iodevice);

    if (!active && _audio->state() == QAudio::ActiveState)
        _audio->stop();

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
void Measure::averageRealloc(bool force)
{
    if (!force && _average == _setAverage)
        return;

    if (averageData)        delete[] averageData;
    if (averageReference)   delete[] averageReference;
    if (averageDeconvolution) delete[] averageDeconvolution;

    averageData      = new complex*[_setAverage];
    averageReference = new complex*[_setAverage];
    averageDeconvolution = new float*[_setAverage];

    for (int i = 0; i < _setAverage; i ++) {
        averageData[i]      = new complex[_dataLength];
        averageReference[i] = new complex[_dataLength];
        averageDeconvolution[i] = new float[_deconvolutionSize];
    }

    //aply new value
    _average = _setAverage;
}
int Measure::sampleRate() const
{
    return _audio->format().sampleRate();
}
qint64 Measure::writeData(const char *data, qint64 len)
{
    const int channelBytes = _format.sampleSize() / 8;
    const int sampleBytes  = _format.channelCount() * channelBytes;
    const int numSamples   = len / sampleBytes;

    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    const float *d;

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < _format.channelCount(); ++j) {

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

        //delta
        //dataFT->change(dataStack->current(), referenceStack->current());

        //fast
        _dataFT->add(dataStack->current(), referenceStack->current());

        //deconvolution
        _deconv->add(referenceStack->current(), dataStack->current());
    }
    _dataFT->fast(_window);
    _deconv->transform();
    averaging();

    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
QTimer *Measure::getTimer() const
{
    return _timer;
}

void Measure::setTimer(QTimer *value)
{
    _timer = value;
}

void Measure::averaging()
{
    averageRealloc();

    _avgcounter ++;
    if (_avgcounter >= _average) _avgcounter = 0;

    for (int i = 0; i < _dataLength ; i++) {
        averageData[_avgcounter][i]      = _dataFT->af(_ftdata[i].fftPoint);
        averageReference[_avgcounter][i] = _dataFT->bf(_ftdata[i].fftPoint);

        _ftdata[i].data      = 0.0;
        _ftdata[i].reference = 0.0;

        for (int j = 0; j < _average; j++) {
            _ftdata[i].data      += averageData[j][i];
            _ftdata[i].reference += averageReference[j][i];
        }
        _ftdata[i].data      /= _average * _window->gain();
        _ftdata[i].reference /= _average * _window->gain();
    }

    for (int i = 0, j = _deconvolutionSize / 2; i < _deconvolutionSize; i++, j++) {
        averageDeconvolution[_avgcounter][i]  = _deconv->get(i);
        impulseData[i] = 0.0;
        for (int j = 0; j < _average; j++) {
            impulseData[i] += averageDeconvolution[j][i];
        }
        impulseData[i] /= (float)_average;

        if (j > _deconvolutionSize) j -= _deconvolutionSize;
        _impulseData[j].value = impulseData[i];
        _impulseData[j].time  = (j - _deconvolutionSize / 2) * 1000.0 / sampleRate();
    }
}
QObject *Measure::store()
{
//    Stored *store = new Stored(this);
//    store->setSampleRate(sampleRate());
//    store->build(this);

//    return store;
}
