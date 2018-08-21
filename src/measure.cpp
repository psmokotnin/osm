#include <algorithm>
#include "measure.h"

Measure::Measure(QObject *parent) : Fftchart::Source(parent),
    dataMeter(12000),
    referenceMeter(12000)
{
    _iodevice = new InputDevice(this);
    connect(_iodevice, SIGNAL(recived(const char *, qint64)), SLOT(writeData(const char *, qint64)));

    _fftPower = 14;//16 - 65K 0.73Hz;
    _fftSize = static_cast<unsigned int>(pow(2, _fftPower));
    _deconvolutionSize = static_cast<unsigned int>(pow(2, 12));

    _dataFT  = new FourierTransform(fftSize());
    _window = new WindowFunction(fftSize());
    _window->setType(WindowFunction::Type::hann);

    QAudioDeviceInfo device = QAudioDeviceInfo::defaultInputDevice();
    selectDevice(device.deviceName());

    calculateDataLength();
    _dataFT->prepareFast();

    _deconv = new Deconvolution(_deconvolutionSize);
    _impulseData = new TimeData[_deconvolutionSize];

    impulseData = new complex[_deconvolutionSize];

    dataStack = new AudioStack(_fftSize);
    referenceStack = new AudioStack(_fftSize);

    setAverage(1);
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
    if (name == deviceName())
        return;

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (name == deviceInfo.deviceName()) {
            if (_audio) {
                _audio->stop();
                delete _audio;
            }
            _iodevice->close();

            _device = deviceInfo;

            _chanelCount = std::max(_dataChanel, _referenceChanel) + 1;
            foreach (auto c, _device.supportedChannelCounts()) {
                unsigned int formatChanels = static_cast<unsigned int>(c);
                if (formatChanels > _chanelCount)
                    _chanelCount = formatChanels;
            }

            _format.setSampleRate(48000);
            _format.setChannelCount(static_cast<int>(_chanelCount));
            _format.setSampleSize(32);
            _format.setCodec("audio/pcm");
            _format.setByteOrder(QAudioFormat::LittleEndian);
            _format.setSampleType(QAudioFormat::Float);

            _audio = new QAudioInput(_device, _format, this);
            _iodevice->open(InputDevice::WriteOnly);
            _audio->setBufferSize(65536 * static_cast<int>(_chanelCount));
            if (active()) {
                _audio->start(_iodevice);
                _chanelCount = static_cast<unsigned int>(_format.channelCount());
            }
            break;
        }
    }
}
void Measure::setFftPower(unsigned int power)
{
    if (_fftPower != power) {
        _audio->suspend();

        _fftPower = power;
        _fftSize  = static_cast<unsigned int>(pow(2, _fftPower));

        _dataFT->setSize(_fftSize);
        _dataFT->prepareFast();
        _window->setSize(_fftSize);

        calculateDataLength();

        averageRealloc(true);

        _audio->resume();
    }
}
void Measure::calculateDataLength()
{        
    _dataLength = _fftSize / 2;
    _ftdata = new FTData[_dataLength];
    for (unsigned int i = 0; i < _dataLength; i++) {
        _ftdata[i].frequency = static_cast<float>(i * sampleRate()) / _fftSize;
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
void Measure::setDelay(unsigned long delay)
{
    long delta = static_cast<long>(_delay) - static_cast<long>(delay);
    _delay = delay;
    referenceStack->setSize(fftSize() + _delay);
    referenceStack->rewind(delta);
}
void Measure::setAverage(unsigned int average)
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
    if (estimatedDelays)    delete[] estimatedDelays;
    if (dataLPFs)           delete[] dataLPFs;
    if (referenceLPFs)      delete[] referenceLPFs;

    averageData      = new complex*[_setAverage];
    averageReference = new complex*[_setAverage];
    averageDeconvolution = new float*[_setAverage];
    estimatedDelays  = new unsigned long[_setAverage];

    for (unsigned int i = 0; i < _setAverage; i ++) {
        averageData[i]      = new complex[_dataLength];
        averageReference[i] = new complex[_dataLength];
        averageDeconvolution[i] = new float[_deconvolutionSize];
        estimatedDelays[i]  = 0;
    }
    dataLPFs      = new Filter[_dataLength];
    referenceLPFs = new Filter[_dataLength];

    //aply new value
    _average = _setAverage;
}
unsigned int Measure::sampleRate() const
{
    return static_cast<unsigned int>(_audio->format().sampleRate());
}
qint64 Measure::writeData(const char *data, qint64 len)
{
    const auto channelBytes = _format.sampleSize() / 8;
    const auto sampleBytes  = _format.channelCount() * channelBytes;
    const auto numSamples   = len / sampleBytes;

    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    const float *d;

    for (auto i = 0; i < numSamples; ++i) {
        for (unsigned int j = 0; j < static_cast<unsigned int>(_format.channelCount()); ++j) {

            d = reinterpret_cast<const float*>(ptr);
            if (j == _dataChanel) {
                dataStack->add((_polarity ? -1 * *d : *d));
                dataMeter.add(*d);
            }

            if (j == _referenceChanel) {
                referenceStack->add(*d);
                referenceMeter.add(*d);
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

        _dataFT->add(dataStack->current(), referenceStack->current());

        //deconvolution
        _deconv->add(referenceStack->current(), dataStack->current());
    }
    _dataFT->fast(_window);
    _deconv->transform();
    averaging();
    _level = dataMeter.value();
    _referenceLevel = referenceMeter.value();

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

    for (unsigned int i = 0; i < _dataLength ; i++) {
        if (_lpf) {
            averageData[_avgcounter][i]      = dataLPFs[i](     _dataFT->af(i));
            averageReference[_avgcounter][i] = referenceLPFs[i](_dataFT->bf(i));
        } else {
            averageData[_avgcounter][i]      = _dataFT->af(i);
            averageReference[_avgcounter][i] = _dataFT->bf(i);
        }

        _ftdata[i].data      = 0.0;
        _ftdata[i].reference = 0.0;

        for (unsigned int j = 0; j < _average; j++) {
            _ftdata[i].data      += averageData[j][i];
            _ftdata[i].reference += averageReference[j][i];
        }
        _ftdata[i].data      /= _average * _window->gain();
        _ftdata[i].reference /= _average * _window->gain();
    }

    int t = 0;
    for (unsigned int i = 0, j = _deconvolutionSize / 2; i < _deconvolutionSize; i++, j++, t++) {
        averageDeconvolution[_avgcounter][i]  = _deconv->get(i);
        impulseData[i] = 0.0;
        for (unsigned int k = 0; k < _average; k++) {
            impulseData[i] += averageDeconvolution[k][i];
        }
        impulseData[i] /= static_cast<float>(_average);

        if (t > static_cast<int>(_deconvolutionSize / 2)) {
            t -= static_cast<int>(_deconvolutionSize);
            j -= _deconvolutionSize;
        }
        _impulseData[j].value = impulseData[i];
        _impulseData[j].time  = static_cast<float>(t * 1000.0 / sampleRate());//ms
    }

    estimatedDelays[_avgcounter] = _deconv->maxPoint();
    _estimatedDelay = 0;
    for (unsigned int k = 0; k < _average; k++) {
        _estimatedDelay += estimatedDelays[k];
    }
    _estimatedDelay /= _average;
    emit estimatedChanged();
}
QObject *Measure::store()
{
    Stored *store = new Stored(this);
    store->build(this);

    return store;
}
long Measure::estimated() const noexcept
{
    if (_estimatedDelay > _deconvolutionSize / 2) {
        return _estimatedDelay - _deconvolutionSize + static_cast<long>(_delay);
    }
    return _estimatedDelay + static_cast<long>(_delay);
}
