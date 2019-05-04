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

Measurement::Measurement(Settings *settings, QObject *parent) : Fftchart::Source(parent),
    m_timer(nullptr), m_timerThread(nullptr),
    m_audioThread(nullptr),
    m_settings(settings),
    m_average(1),
    m_delay(0), m_setDelay(0),
    m_estimatedDelay(0),
    m_polarity(false), m_error(false),
    dataMeter(12000), referenceMeter(12000), //250ms
    m_window(WindowFunction::Type::hann, this),
    m_averageType(AverageType::LPF),
    m_filtersFrequency(Filter::Frequency::FOURTHHZ),
    _fftPower(1), _setfftPower(1)
{
    _name = "Measurement";
    setObjectName(_name);

    connect(&m_audioThread, SIGNAL(recived(const QByteArray&)), SLOT(writeData(const QByteArray&)), Qt::DirectConnection);
    connect(&m_audioThread, SIGNAL(deviceChanged(QString)), this, SIGNAL(deviceChanged(QString)));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SIGNAL(chanelsCountChanged()));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SLOT(recalculateDataLength()));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SIGNAL(sampleRateChanged()));
    connect(&m_audioThread, SIGNAL(deviceError()), this, SLOT(setError()));

    QAudioDeviceInfo device(QAudioDeviceInfo::defaultInputDevice());

    setName(        m_settings->reactValue<Measurement, QString>(           "name",         this,   &Measurement::nameChanged,          name()).toString());
    setColor(       m_settings->reactValue<Measurement, QColor>(            "color",        this,   &Measurement::colorChanged,         color()).value<QColor>());
    setDelay(       m_settings->reactValue<Measurement, unsigned int>(      "delay",        this,   &Measurement::delayChanged,         delay()).toUInt());
    setAverageType( m_settings->reactValue<Measurement, AverageType>(       "average/type", this,   &Measurement::averageTypeChanged,   averageType()));
    setAverage(     m_settings->reactValue<Measurement, unsigned int>(      "average/fifo", this,   &Measurement::averageChanged,       average()).toUInt());
    setFiltersFrequency(m_settings->reactValue<Measurement, Filter::Frequency>("average/lpf", this, &Measurement::filtersFrequencyChanged, filtersFrequency()));
    setWindowType(  m_settings->reactValue<Measurement, WindowFunction::Type>("window",     this,   &Measurement::windowTypeChanged,    m_window.type()));
    setDataChanel(  m_settings->reactValue<Measurement, unsigned int>(      "route/data",   this,   &Measurement::dataChanelChanged,    dataChanel()).toUInt());
    setReferenceChanel(m_settings->reactValue<Measurement, unsigned int>(   "route/reference", this,&Measurement::referenceChanelChanged, referenceChanel()).toUInt());
    setPolarity(    m_settings->reactValue<Measurement, bool>(              "polarity",     this,   &Measurement::polarityChanged,      polarity()).toBool());
    if (!selectDevice(   m_settings->reactValue<Measurement, QString>(           "device",       this,   &Measurement::deviceChanged,        device.deviceName()).toString())) {
        selectDevice(device);
    }

    _setfftPower = _fftPower = m_settings->reactValue<Measurement, unsigned int>(
                   "fftpower", this, &Measurement::fftPowerChanged, 14).toUInt();
    _fftSize = static_cast<unsigned int>(pow(2, _fftPower));
    m_deconvolutionSize = static_cast<unsigned int>(pow(2, 12));

    m_dataFT.setSize(fftSize());
    m_window.setSize(fftSize());

    calculateDataLength();
    m_dataFT.prepareFast();
    m_moduleLPFs.resize(_dataLength);
    m_magnitudeLPFs.resize(_dataLength);
    m_phaseLPFs.resize(_dataLength);

    m_deconvolution.setSize(m_deconvolutionSize);
    _impulseData = new TimeData[m_deconvolutionSize];
    m_deconvLPFs.resize(m_deconvolutionSize);

    pahseAvg.setSize(fftSize());
    moduleAvg.setSize(fftSize());
    magnitudeAvg.setSize(fftSize());
    deconvAvg.setSize(m_deconvolutionSize);
    deconvAvg.reset();
    m_coherence.setSize(fftSize());
    m_coherence.setDepth(Filter::BesselLPF<float>::ORDER);

    m_timer.setInterval(80);//12.5 per sec
    m_timer.moveToThread(&m_timerThread);
    connect(&m_timer, SIGNAL(timeout()), SLOT(transform()), Qt::DirectConnection);
    connect(&m_timerThread, SIGNAL(started()), &m_timer, SLOT(start()), Qt::DirectConnection);
    connect(&m_timerThread, SIGNAL(finished()), &m_timer, SLOT(stop()), Qt::DirectConnection);
    m_timerThread.start();
    setActive(true);
}
Measurement::~Measurement()
{
    m_audioThread.quit();
    m_audioThread.wait();

    m_timerThread.quit();
    m_timerThread.wait();
}
QVariant Measurement::getDeviceList() const
{
    QStringList deviceList;
    QAudioFormat format44, format48;
    format48.setChannelCount(2);
    format48.setSampleRate(48000);
    format48.setSampleSize(32);
    format48.setCodec("audio/pcm");
    format48.setByteOrder(QAudioFormat::LittleEndian);
    format48.setSampleType(QAudioFormat::Float);

    format44 = format48;
    format44.setSampleRate(44100);

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (deviceInfo.isFormatSupported(format44) || deviceInfo.isFormatSupported(format48)) {
            deviceList << deviceInfo.deviceName();
        }
    }
    return QVariant::fromValue(deviceList);
}
QString Measurement::deviceName() const
{
    return m_audioThread.device().deviceName();
}
bool Measurement::selectDevice(const QString &name)
{
    if (name == deviceName())
        return true;

    QStringList devices = getDeviceList().value<QStringList>();
    if (devices.indexOf(name) == -1) {
        return false;
    }
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (name == deviceInfo.deviceName()) {
            selectDevice(deviceInfo);
            return true;
        }
    }
    return false;
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
void Measurement::setDataChanel(unsigned int n)
{
    if (n != dataChanel()) {
        m_audioThread.setDataChanel(n);
        emit dataChanelChanged(dataChanel());
    }
}
void Measurement::setReferenceChanel(unsigned int n)
{
    if (n != referenceChanel()) {
        m_audioThread.setReferenceChanel(n);
        emit referenceChanelChanged(referenceChanel());
    }
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
        m_coherence.setSize(fftSize());

        calculateDataLength();
        m_moduleLPFs.resize(_dataLength);
        m_magnitudeLPFs.resize(_dataLength);
        m_phaseLPFs.resize(_dataLength);

        emit fftPowerChanged(_fftPower);
    }
}
void Measurement::setFiltersFrequency(Filter::Frequency frequency)
{
    if (m_filtersFrequency != frequency) {
        m_filtersFrequency = frequency;

        auto setFrequency = [&m_filtersFrequency = m_filtersFrequency](Filter::BesselLPF<float> *f) {
            f->setFrequency(m_filtersFrequency);
        };

        m_moduleLPFs.each(setFrequency);
        m_magnitudeLPFs.each(setFrequency);
        m_deconvLPFs.each(setFrequency);
        m_phaseLPFs.each([&m_filtersFrequency = m_filtersFrequency](Filter::BesselLPF<complex> *f) {
            f->setFrequency(m_filtersFrequency);
        });

        filtersFrequencyChanged(m_filtersFrequency);
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
    if (active == _active)
        return;

    Fftchart::Source::setActive(active);
    m_error = false;
    emit errorChanged(m_error);

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
void Measurement::setError()
{
    Fftchart::Source::setActive(false);
    m_error = true;
    dataMeter.reset();
    referenceMeter.reset();
    emit errorChanged(m_error);
    emit levelChanged();
    emit referenceLevelChanged();
}
//this calls from gui thread
void Measurement::setDelay(unsigned int delay)
{
    m_setDelay = delay;
}
//this calls from timer thread
void Measurement::updateDelay()
{
    if (m_delay != m_setDelay) {
        long delta = static_cast<long>(m_delay) - static_cast<long>(m_setDelay);
        m_delay = m_setDelay;
        bool direction = std::signbit(delta);
        delta = std::abs(delta);
        for (long i = 0; i != delta; ++i) {
            if (direction) {
                reference.push(0.f);
            } else {
                data.push(0.f);
            }
        }
        emit delayChanged(m_delay);
    }
}
void Measurement::setAverage(unsigned int average)
{
    if (m_average != average) {
        std::lock_guard<std::mutex> guard(dataMutex);
        m_average = average;
        deconvAvg.setDepth(m_average);
        moduleAvg.setDepth(m_average);
        magnitudeAvg.setDepth(m_average);
        pahseAvg.setDepth(m_average);
        averageChanged(m_average);
    }
}
void Measurement::setPolarity(bool polarity)
{
    if (m_polarity != polarity) {
        m_polarity = polarity;
        emit polarityChanged(m_polarity);
    }
}
void Measurement::setAverageType(AverageType type)
{
    if (m_averageType != type) {
        std::lock_guard<std::mutex> guard(dataMutex);
        m_averageType = type;
        emit averageTypeChanged(m_averageType);
    }
}
unsigned int Measurement::sampleRate() const
{
    return static_cast<unsigned int>(m_audioThread.sampleRate());
}
void Measurement::setWindowType(WindowFunction::Type type)
{
    if (m_window.type() != type) {
        m_window.setType(type);
        moduleAvg.setGain(m_window.gain());
        emit windowTypeChanged(m_window.type());
    }
}
void Measurement::writeData(const QByteArray& buffer)
{
    float sample;
    auto totalChanels = static_cast<unsigned int>(m_audioThread.format().channelCount());
    unsigned int currentChanel = 0;
    std::lock_guard<std::mutex> guard(dataMutex);
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {

        if (currentChanel == dataChanel()) {
            memcpy(&sample, it, sizeof(float));
            data.pushnpop((m_polarity ? -1 * sample : sample), 48000);
            dataMeter.add(sample);
        }

        if (currentChanel == referenceChanel()) {
            memcpy(&sample, it, sizeof(float));
            reference.pushnpop(sample, 48000);
            referenceMeter.add(sample);
        }
        ++currentChanel;
        if (currentChanel >= totalChanels) {
            currentChanel = 0;
        }
        it += 3;
    }
}
void Measurement::transform()
{
    if (!_active || m_error)
        return;

    lock();
    updateFftPower();
    updateDelay();

    float d, r;
    while (data.size() > 0 && reference.size() > 0) {
        d = data.pop();
        r = reference.pop();

        m_dataFT.add(d, r);
        m_deconvolution.add(r, d);
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
        if (std::isnan(magnitude) || std::isinf(magnitude)) {
#endif
            magnitude = 0.f;
        }
        p.polar(m_dataFT.bf(i).arg() - m_dataFT.af(i).arg());

        switch (averageType()) {
            case AverageType::OFF:
                _ftdata[i].magnitude = magnitude;
                _ftdata[i].module    = m_dataFT.af(i).abs();
                _ftdata[i].phase     = p;
            break;

            case AverageType::LPF:
                _ftdata[i].magnitude = m_magnitudeLPFs[i](magnitude);
                _ftdata[i].module    = m_moduleLPFs[i](m_dataFT.af(i).abs());
                _ftdata[i].phase     = m_phaseLPFs[i](p);
            break;

            case AverageType::FIFO:
                magnitudeAvg.append(i, magnitude );
                moduleAvg.append(i,    m_dataFT.af(i).abs() );
                pahseAvg.append(i,     p);

                _ftdata[i].magnitude = magnitudeAvg.value(i);
                _ftdata[i].module    = moduleAvg.value(i);
                _ftdata[i].phase     = pahseAvg.value(i);
            break;
        }

        m_coherence.append(i, m_dataFT.bf(i), m_dataFT.af(i));
        _ftdata[i].coherence = m_coherence.value(i);
    }

    int t = 0;
    float kt = 1000.f / sampleRate();
    float max(0.f);
    for (unsigned int i = 0, j = m_deconvolutionSize / 2 - 1; i < m_deconvolutionSize; i++, j++, t++) {

        if (t > static_cast<int>(m_deconvolutionSize / 2)) {
            t -= static_cast<int>(m_deconvolutionSize);
            j -= m_deconvolutionSize;
        }

        switch (averageType()) {
            case AverageType::OFF:
                _impulseData[j].value.real = m_deconvolution.get(i);
            break;
            case AverageType::LPF:
                _impulseData[j].value.real = m_deconvLPFs[i](m_deconvolution.get(i));
            break;
            case AverageType::FIFO:
                deconvAvg.append(i, m_deconvolution.get(i));
                _impulseData[j].value.real = deconvAvg.value(i);
            break;
        }

        if (max < abs(_impulseData[j].value.real)) {
            max = abs(_impulseData[j].value.real);
            m_estimatedDelay = i;
        }
        _impulseData[j].time  = t * kt;//ms
    }
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
