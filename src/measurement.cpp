/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QThread>
#include <algorithm>
#include "measurement.h"

Measurement::Measurement(QObject *parent) : Fftchart::Source(parent),
    dataMeter(12000),
    referenceMeter(12000)
{
    _name = "Measurement";
    setObjectName(_name);
    _iodevice = new InputDevice(this);
    connect(_iodevice, SIGNAL(recived(const char *, qint64)), SLOT(writeData(const char *, qint64)));

    _fftPower = 14;//16 - 65K 0.73Hz;
    _fftSize = static_cast<unsigned int>(pow(2, _fftPower));
    _deconvolutionSize = static_cast<unsigned int>(pow(2, 12));

    _dataFT  = new FourierTransform(fftSize());
    _window = new WindowFunction(fftSize());
    _window->setType(WindowFunction::Type::hann);

    QAudioDeviceInfo device(QAudioDeviceInfo::defaultInputDevice());
    selectDevice(device);

    calculateDataLength();
    _dataFT->prepareFast();

    _deconv = new Deconvolution(_deconvolutionSize);
    _impulseData = new TimeData[_deconvolutionSize];

    impulseData = new complex[_deconvolutionSize];

    dataStack = new AudioStack(_fftSize);
    referenceStack = new AudioStack(_fftSize);

    setAverage(1);
    m_timerThread = new QThread(this);
    _timer = new QTimer(nullptr);
    _timer->setInterval(80);//12.5 per sec
    _timer->moveToThread(m_timerThread);
    connect(_timer, SIGNAL(timeout()), SLOT(transform()), Qt::DirectConnection);
    connect(m_timerThread, SIGNAL(started()), _timer, SLOT(start()), Qt::DirectConnection);
}
Measurement::~Measurement()
{
    _audio->stop();
    m_timerThread->quit();
    m_timerThread->wait();
    delete(m_timerThread);
}
QVariant Measurement::getDeviceList(void)
{
    QStringList deviceList;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        deviceList << deviceInfo.deviceName();
    }
    return QVariant::fromValue(deviceList);
}
QString Measurement::deviceName()
{
    return _device.deviceName();
}
void Measurement::selectDevice(QString name)
{
    if (name == deviceName())
        return;

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (name == deviceInfo.deviceName()) {
            selectDevice(deviceInfo);
        }
    }
}
void Measurement::selectDevice(QAudioDeviceInfo deviceInfo)
{
    if (_audio) {
        _audio->stop();
        delete _audio;
    }
    _iodevice->close();
    _maxChanelCount = 0;

    _device = deviceInfo;

    _chanelCount = std::max(_dataChanel, _referenceChanel) + 1;
    foreach (auto c, _device.supportedChannelCounts()) {
        unsigned int formatChanels = static_cast<unsigned int>(c);
        if (formatChanels > _chanelCount)
            _chanelCount = formatChanels;
        _maxChanelCount = std::max(formatChanels, _maxChanelCount);
    }

    _format.setSampleRate(48000);
    _format.setChannelCount(static_cast<int>(_chanelCount));
    _format.setSampleSize(32);
    _format.setCodec("audio/pcm");
    _format.setByteOrder(QAudioFormat::LittleEndian);
    _format.setSampleType(QAudioFormat::Float);

    _audio = new QAudioInput(_device, _format, this);
    _audio->setBufferSize(16384);
    _iodevice->open(InputDevice::WriteOnly);
    if (active()) {
        _audio->start(_iodevice);
        _chanelCount = static_cast<unsigned int>(_format.channelCount());
    }
    emit chanelsCountChanged();
}
void Measurement::setFftPower(unsigned int power)
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
void Measurement::calculateDataLength()
{        
    _dataLength = _fftSize / 2;
    _ftdata = new FTData[_dataLength];
    for (unsigned int i = 0; i < _dataLength; i++) {
        _ftdata[i].frequency = static_cast<float>(i * sampleRate()) / _fftSize;
    }
}
void Measurement::setActive(bool active)
{
    Fftchart::Source::setActive(active);

    if (active && (
                _audio->state() == QAudio::IdleState ||
                _audio->state() == QAudio::StoppedState)
       )
        _audio->start(_iodevice);

    if (!active && _audio->state() == QAudio::ActiveState)
        _audio->stop();

    _level  = -INFINITY;
    _referenceLevel = -INFINITY;
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measurement::setDelay(unsigned long delay)
{
    long delta = static_cast<long>(_delay) - static_cast<long>(delay);
    _delay = delay;
    referenceStack->setSize(fftSize() + _delay);
    referenceStack->rewind(delta);
}
void Measurement::setAverage(unsigned int average)
{
    _setAverage = average;
}
void Measurement::averageRealloc(bool force)
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
unsigned int Measurement::sampleRate() const
{
    return static_cast<unsigned int>(_audio->format().sampleRate());
}
qint64 Measurement::writeData(const char *data, qint64 len)
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
    if (!m_timerThread->isRunning()) {
        m_timerThread->start();
    }
    return len;
}
void Measurement::transform()
{
    if (!_active)
        return;

    _level = _referenceLevel = 0.0;
    lock();
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
    unlock();

    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
QTimer *Measurement::getTimer() const
{
    return _timer;
}

void Measurement::setTimer(QTimer *value)
{
    _timer = value;
}

void Measurement::averaging()
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
QObject *Measurement::store()
{
    Stored *store = new Stored(this);
    store->build(this);

    return store;
}
long Measurement::estimated() const noexcept
{
    if (_estimatedDelay > _deconvolutionSize / 2) {
        return _estimatedDelay - _deconvolutionSize + static_cast<long>(_delay);
    }
    return _estimatedDelay + static_cast<long>(_delay);
}
