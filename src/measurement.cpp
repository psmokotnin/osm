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
    m_timer(nullptr), m_timerThread(nullptr),
    m_audioThread(nullptr),
    m_average(0),
    m_delay(0), m_setDelay(0),
    m_estimatedDelay(0),
    m_polarity(false), m_lpf(true),
    dataMeter(12000), referenceMeter(12000), //250ms
    m_window(WindowFunction::Type::hann)
{
    _name = "Measurement";
    setObjectName(_name);

    connect(&m_audioThread, SIGNAL(recived(const QByteArray&)), SLOT(writeData(const QByteArray&)), Qt::DirectConnection);
    connect(&m_audioThread, SIGNAL(deviceChanged()), this, SIGNAL(deviceChanged()));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SIGNAL(chanelsCountChanged()));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SLOT(recalculateDataLength()));
    QObject::connect(&m_audioThread, &MeasurementAudioThread::formatChanged,
                     [&]{
        this->setSampleRate(static_cast<unsigned int>(m_audioThread.format().sampleRate()));
    });

    _setfftPower = _fftPower = 14;//16 - 65K 0.73Hz;
    _fftSize = static_cast<unsigned int>(pow(2, _fftPower));
    m_deconvolutionSize = static_cast<unsigned int>(pow(2, 12));

    m_dataFT.setSize(fftSize());
    m_window.setSize(fftSize());

    QAudioDeviceInfo device(QAudioDeviceInfo::defaultInputDevice());
    selectDevice(device);

    calculateDataLength();
    m_dataFT.prepareFast();
    m_moduleLPFs.resize(_dataLength);
    m_magnitudeLPFs.resize(_dataLength);
    m_phaseLPFs.resize(_dataLength);

    m_deconvolution.setSize(m_deconvolutionSize);
    _impulseData = new TimeData[m_deconvolutionSize];
    m_deconvLPFs.resize(m_deconvolutionSize);

    dataStack = new AudioStack(_fftSize);
    referenceStack = new AudioStack(_fftSize);

    pahseAvg.setSize(fftSize());
    moduleAvg.setSize(fftSize());
    magnitudeAvg.setSize(fftSize());
    deconvAvg.setSize(m_deconvolutionSize);
    deconvAvg.reset();
    estimatedDelayAvg.setSize(1);
    estimatedDelayAvg.reset();

    setAverage(1);
    m_timer.setInterval(80);//12.5 per sec
    m_timer.moveToThread(&m_timerThread);
    connect(&m_timer, SIGNAL(timeout()), SLOT(transform()), Qt::DirectConnection);
    connect(&m_timerThread, SIGNAL(started()), &m_timer, SLOT(start()), Qt::DirectConnection);
    connect(&m_timerThread, SIGNAL(finished()), &m_timer, SLOT(stop()), Qt::DirectConnection);
    m_timerThread.start();
}
Measurement::~Measurement()
{
    m_audioThread.quit();
    m_audioThread.wait();

    m_timerThread.quit();
    m_timerThread.wait();

    delete dataStack;
    delete referenceStack;
}
QVariant Measurement::getDeviceList() const
{
    QStringList deviceList;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        deviceList << deviceInfo.deviceName();
    }
    return QVariant::fromValue(deviceList);
}
QString Measurement::deviceName() const
{
    return m_audioThread.device().deviceName();
}
void Measurement::selectDevice(const QString &name)
{
    if (name == deviceName())
        return;

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (name == deviceInfo.deviceName()) {
            selectDevice(deviceInfo);
        }
    }
}
void Measurement::selectDevice(const QAudioDeviceInfo &deviceInfo)
{
    QMetaObject::invokeMethod(
                &m_audioThread,
                "selectDevice",
                Qt::QueuedConnection,
                Q_ARG(QAudioDeviceInfo, deviceInfo),
                Q_ARG(bool, active())
                );
}
//this calls from gui thread
void Measurement::setFftPower(unsigned int power)
{
    if (_fftPower != power) {
        _setfftPower = power;
    }
}
//this calls from timer thread
//should be called while mutex locked
void Measurement::updateFftPower()
{
    if (_fftPower != _setfftPower) {
        _fftPower = _setfftPower;
        _fftSize  = static_cast<unsigned int>(pow(2, _fftPower));

        m_dataFT.setSize(_fftSize);
        m_dataFT.prepareFast();
        m_window.setSize(_fftSize);

        moduleAvg.setSize(_fftSize);
        magnitudeAvg.setSize(_fftSize);
        pahseAvg.setSize(_fftSize);

        calculateDataLength();
        m_moduleLPFs.resize(_dataLength);
        m_magnitudeLPFs.resize(_dataLength);
        m_phaseLPFs.resize(_dataLength);

        emit fftPowerChanged(_fftPower);
    }
}
void Measurement::recalculateDataLength()
{
    std::lock_guard<std::mutex> guard(dataMutex);
    calculateDataLength();
}
void Measurement::calculateDataLength()
{        
    _dataLength = _fftSize / 2;
    if (_ftdata)
        delete[] _ftdata;
    _ftdata = new FTData[_dataLength];

    float kf = static_cast<float>(sampleRate()) / _fftSize;
    for (unsigned int i = 0; i < _dataLength; ++i) {
        _ftdata[i].frequency = static_cast<float>(i * kf);
    }
}
void Measurement::setActive(bool active)
{
    Fftchart::Source::setActive(active);

    QMetaObject::invokeMethod(
                &m_audioThread,
                "setActive",
                Qt::QueuedConnection,
                Q_ARG(bool, active)
                );
    dataMeter.reset();
    referenceMeter.reset();
    emit levelChanged();
    emit referenceLevelChanged();
}
//this calls from gui thread
void Measurement::setDelay(unsigned long delay)
{
    m_setDelay = delay;
}
//this calls from timer thread
void Measurement::updateDelay()
{
    if (m_delay != m_setDelay) {
        long delta = static_cast<long>(m_delay) - static_cast<long>(m_setDelay);
        m_delay = m_setDelay;
        referenceStack->setSize(fftSize() + m_delay);
        referenceStack->rewind(delta);
        emit delayChanged();
    }
}
void Measurement::setAverage(unsigned int average)
{
    m_average = average;
    deconvAvg.setDepth(average);
    estimatedDelayAvg.setDepth(average);
    moduleAvg.setDepth(average);
    magnitudeAvg.setDepth(average);
    pahseAvg.setDepth(average);
}

void Measurement::setWindowType(int t)
{
    m_window.setType(static_cast<WindowFunction::Type>(t));
    moduleAvg.setGain(m_window.gain());
}
void Measurement::writeData(const QByteArray& buffer)
{
    float sample;
    auto totalChanels = static_cast<unsigned int>(m_audioThread.format().channelCount());
    unsigned int currentChanel = 0;
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {

        if (currentChanel == dataChanel()) {
            memcpy(&sample, it, sizeof(float));
            dataStack->add((m_polarity ? -1 * sample : sample));
            dataMeter.add(sample);
        }

        if (currentChanel == referenceChanel()) {
            memcpy(&sample, it, sizeof(float));
            referenceStack->add(sample);
            referenceMeter.add(sample);
        }
        ++currentChanel;
        if (currentChanel >= totalChanels) {
            currentChanel = 0;
        }
        it += 3;
    }
}

void Measurement::setSampleRate(unsigned int sampleRate)
{
    if (m_sampleRate == sampleRate)
        return;

    m_sampleRate = sampleRate;
    emit sampleRateChanged(m_sampleRate);
}

void Measurement::transform()
{
    if (!_active)
        return;

    lock();
    updateFftPower();
    updateDelay();
    while (dataStack->isNext() && referenceStack->isNext()) {
        dataStack->next();
        referenceStack->next();

        m_dataFT.add(dataStack->current(), referenceStack->current());
        m_deconvolution.add(referenceStack->current(), dataStack->current());
    }
    m_dataFT.ufast(&m_window);
    m_deconvolution.transform(&m_window);
    averaging();
    unlock();

    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measurement::averaging()
{
    complex p;
    for (unsigned int i = 0; i < _dataLength ; i++) {

        float magnitude = m_dataFT.af(i).abs() / m_dataFT.bf(i).abs();
#ifdef WIN64
        if (magnitude/0.f == magnitude) {
#else
        if (std::isnan(magnitude)) {
#endif
            magnitude = 0.f;
        }
        magnitudeAvg.append(i,  (m_lpf ? m_magnitudeLPFs[i](magnitude)         : magnitude ));
        moduleAvg.append(i,     (m_lpf ? m_moduleLPFs[i](m_dataFT.af(i).abs()) : m_dataFT.af(i).abs() ));

        p.polar(m_dataFT.bf(i).arg() - m_dataFT.af(i).arg());
        pahseAvg.append(i,      (m_lpf ? m_phaseLPFs[i](p) : p ));

        _ftdata[i].magnitude = magnitudeAvg.value(i);
        _ftdata[i].module    = moduleAvg.value(i);
        _ftdata[i].phase     = pahseAvg.value(i).arg();
    }

    int t = 0;
    float kt = 1000.f / sampleRate();
    for (unsigned int i = 0, j = m_deconvolutionSize / 2 - 1; i < m_deconvolutionSize; i++, j++, t++) {

        deconvAvg.append(i, m_deconvolution.get(i));

        if (t > static_cast<int>(m_deconvolutionSize / 2)) {
            t -= static_cast<int>(m_deconvolutionSize);
            j -= m_deconvolutionSize;
        }
        _impulseData[j].value.real = (m_lpf ? m_deconvLPFs[i](deconvAvg.value(i)) : deconvAvg.value(i));
        _impulseData[j].time  = t * kt;//ms
    }
    estimatedDelayAvg.append(0, m_deconvolution.maxPoint());
    m_estimatedDelay = estimatedDelayAvg.value(0);
    emit estimatedChanged();
}
QObject *Measurement::store()
{
    auto *store = new Stored(this);
    store->build(this);

    return store;
}
long Measurement::estimated() const noexcept
{
    if (m_estimatedDelay > m_deconvolutionSize / 2) {
        return m_estimatedDelay - m_deconvolutionSize + static_cast<long>(m_delay);
    }
    return m_estimatedDelay + static_cast<long>(m_delay);
}
