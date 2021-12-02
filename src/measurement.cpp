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
#include <QFile>
#include <QUrl>
#include <QJsonArray>
#include <QDateTime>
#include <QtMath>
#include <algorithm>
#include "measurement.h"
#include "audio/client.h"
#include "generator/generatorthread.h"

const std::map<Measurement::Mode, QString>Measurement::m_modeMap = {
    {Measurement::FFT10, "10"},
    {Measurement::FFT12, "12"},
    {Measurement::FFT14, "14"},
    {Measurement::FFT15, "15"},
    {Measurement::FFT16, "16"},
    {Measurement::LFT,   "LTW"}
};
const std::map<Measurement::Mode, int>Measurement::m_FFTsizes = {
    {Measurement::FFT10, 10},
    {Measurement::FFT12, 12},
    {Measurement::FFT14, 14},
    {Measurement::FFT15, 15},
    {Measurement::FFT16, 16}
};

Measurement::Measurement(Settings *settings, QObject *parent) : chart::Source(parent),
    m_timer(nullptr), m_timerThread(nullptr),
    m_input(this),
    m_deviceId(audio::Client::defaultInputDeviceId()),
    m_audioStream(nullptr),
    m_sampleRate(48000),
    m_settings(settings),
    m_mode(FFT14), m_currentMode(),
    m_dataChanel(0), m_referenceChanel(1),
    m_average(1),
    m_workingDelay(0), m_delay(0), m_delayFinderCounter(0), m_gain(1.f),
    m_estimatedDelay(0),
    m_polarity(false), m_error(false),
    m_dataMeter(12000), m_referenceMeter(12000), //250ms
    m_windowFunctionType(WindowFunction::Type::Hann),
    m_averageType(AverageType::LPF),
    m_filtersFrequency(Filter::Frequency::FourthHz),
    m_enableCalibration(false), m_calibrationLoaded(false), m_calibrationList(), m_calibrationGain()
{
    m_name = "Measurement";
    setObjectName(m_name);

    if (m_settings) {
        setMode(        m_settings->reactValue<Measurement, Mode>(              "mode",         this,
                                                                                &Measurement::modeChanged,          mode()));
        setName(        m_settings->reactValue<Measurement, QString>(           "name",         this,
                                                                                &Measurement::nameChanged,          name()).toString());
        setColor(       m_settings->reactValue<Measurement, QColor>(            "color",        this,
                                                                                &Measurement::colorChanged,         color()).value<QColor>());
        setDelay(       m_settings->reactValue<Measurement, unsigned int>(      "delay",        this,
                                                                                &Measurement::delayChanged,         delay()).toUInt());
        setGain(        m_settings->reactValue<Measurement, float>(             "gain",         this,
                                                                                &Measurement::gainChanged,          gain()).toFloat());
        setAverageType( m_settings->reactValue<Measurement, AverageType>(       "average/type", this,
                                                                                &Measurement::averageTypeChanged,   averageType()));
        setAverage(     m_settings->reactValue<Measurement, unsigned int>(      "average/fifo", this,
                                                                                &Measurement::averageChanged,       average()).toUInt());
        setFiltersFrequency(m_settings->reactValue<Measurement, Filter::Frequency>("average/lpf", this,
                                                                                   &Measurement::filtersFrequencyChanged, filtersFrequency()));
        setWindowType(  m_settings->reactValue<Measurement, WindowFunction::Type>("window",     this,
                                                                                  &Measurement::windowTypeChanged,    m_windowFunctionType));
        setDataChanel(  m_settings->reactValue<Measurement, unsigned int>(      "route/data",   this,
                                                                                &Measurement::dataChanelChanged,    dataChanel()).toUInt());
        setReferenceChanel(m_settings->reactValue<Measurement, unsigned int>(   "route/reference", this,
                                                                                &Measurement::referenceChanelChanged, referenceChanel()).toUInt());
        setPolarity(    m_settings->reactValue<Measurement, bool>(              "polarity",     this,
                                                                                &Measurement::polarityChanged,      polarity()).toBool());
        selectDevice(   m_settings->reactValue<Measurement, QString>(           "device",       this,
                                                                                &Measurement::deviceNameChanged,        deviceName()).toString());
    }
    m_deconvolutionSize = static_cast<unsigned int>(pow(2, 12));

    updateFftPower();
    m_dataFT.setWindowFunctionType(m_windowFunctionType);
    m_moduleLPFs.resize(m_dataLength);
    m_magnitudeLPFs.resize(m_dataLength);
    m_phaseLPFs.resize(m_dataLength);

    m_deconvolution.setSize(m_deconvolutionSize);
    m_deconvolution.setWindowFunctionType(m_windowFunctionType);
    m_delayFinder.setSize(pow(2, 16));
    m_delayFinder.setWindowFunctionType(m_windowFunctionType);
    m_impulseData = new TimeData[m_deconvolutionSize];
    m_deconvLPFs.resize(m_deconvolutionSize);
    m_deconvAvg.setSize(m_deconvolutionSize);
    m_deconvAvg.reset();
    m_coherence.setDepth(21);//Filter::BesselLPF<float>::ORDER);

    m_timer.setInterval(80);//12.5 per sec
    m_timer.moveToThread(&m_timerThread);
    connect(&m_timer, SIGNAL(timeout()), SLOT(transform()), Qt::DirectConnection);
    connect(&m_timerThread, SIGNAL(started()), &m_timer, SLOT(start()), Qt::DirectConnection);
    connect(&m_timerThread, SIGNAL(finished()), &m_timer, SLOT(stop()), Qt::DirectConnection);
    connect(this, &Measurement::audioFormatChanged, this, &Measurement::onSampleRateChanged);
    connect(GeneratorThread::getInstance(), &GeneratorThread::sampleOut, this, &Measurement::newSampleFromGenerator,
            Qt::DirectConnection);
    connect(GeneratorThread::getInstance(), &GeneratorThread::enabledChanged, this, &Measurement::resetLoopBuffer,
            Qt::DirectConnection);

    auto refreshDelays = [this]() {
        m_resetDelay = true;
    };
    connect(this, &Measurement::dataChanelChanged, &m_timer, refreshDelays);
    connect(this, &Measurement::referenceChanelChanged, &m_timer, refreshDelays);
    connect(this, &Measurement::deviceIdChanged, &m_timer, refreshDelays);

    m_timerThread.start();
    setActive(true);
}
Measurement::~Measurement()
{
    setActive(false);

    m_timerThread.quit();
    m_timerThread.wait();
}
QJsonObject Measurement::toJSON(const SourceList *) const noexcept
{
    QJsonObject data;
    data["active"]          = active();
    data["name"]            = name();
    data["delay"]           = static_cast<int>(delay());
    data["gain"]            = static_cast<int>(gain());
    data["averageType"]     = averageType();
    data["average"]         = static_cast<int>(average());
    data["filtersFrequency"] = static_cast<int>(filtersFrequency());
    data["window.type"]     = m_windowFunctionType;
    data["dataChanel"]      = static_cast<int>(dataChanel());
    data["referenceChanel"] = static_cast<int>(referenceChanel());
    data["polarity"]        = polarity();
    data["deviceName"]      = deviceName();
    data["mode"]            = mode();

    QJsonObject color;
    color["red"]    = m_color.red();
    color["green"]  = m_color.green();
    color["blue"]   = m_color.blue();
    color["alpha"]  = m_color.alpha();
    data["color"]   = color;

    QJsonObject calibration;
    calibration["enabled"] = m_enableCalibration;

    QJsonArray calibrationListJson;
    for (auto calibrationRow : m_calibrationList) {
        QJsonArray calibrationRowJson{};
        calibrationRowJson.append(static_cast<double>(calibrationRow[0]));//frequency
        calibrationRowJson.append(static_cast<double>(calibrationRow[1]));//gain
        calibrationRowJson.append(static_cast<double>(calibrationRow[2]));//phase
        calibrationListJson.append(calibrationRowJson);
    }
    calibration["list"] = calibrationListJson;


    data["calibration"] = calibration;
    return data;
}
void Measurement::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    auto castUInt = [](const QJsonValue & value, unsigned int defaultValue = 0) {
        return static_cast<unsigned int>(value.toInt(static_cast<int>(defaultValue)));
    };

    setDelay(            castUInt(data["delay"           ], delay()));
    setGain(             castUInt(data["gain"            ], gain()));
    setAverage(          castUInt(data["average"         ], average()));
    setDataChanel(       castUInt(data["dataChanel"      ], dataChanel()));
    setReferenceChanel(  castUInt(data["referenceChanel" ], referenceChanel()));

    auto jsonColor = data["color"].toObject();
    QColor c(
        jsonColor["red"     ].toInt(0),
        jsonColor["green"   ].toInt(0),
        jsonColor["blue"    ].toInt(0),
        jsonColor["alpha"   ].toInt(1));
    setColor(c);

    setName(             data["name"             ].toString(name()));
    setMode(             data["mode"             ].toInt(mode()));
    setAverageType(      data["averageType"      ].toInt(averageType()));
    setFiltersFrequency( data["filtersFrequency" ].toInt(filtersFrequency()));
    setWindowType(       data["window.type"      ].toInt(m_windowFunctionType));
    setPolarity(         data["polarity"         ].toBool(polarity()));
    selectDevice(        data["deviceName"       ].toString(deviceName()));
    setActive(           data["active"           ].toBool(active()));

    QJsonObject calibration = data["calibration"].toObject();
    if (!calibration.isEmpty()) {
        QJsonArray calibrationListJson = calibration["list"].toArray();
        m_calibrationList.clear();
        for (auto calibrationRowRef : calibrationListJson) {
            auto calibrationRowJson = calibrationRowRef.toArray();
            float
            frequency   = static_cast<float>(calibrationRowJson[0].toDouble(0.0)),
            gain        = static_cast<float>(calibrationRowJson[1].toDouble(0.0)),
            phase       = static_cast<float>(calibrationRowJson[2].toDouble(0.0));
            QVector<float> calibrationData;
            calibrationData << frequency << gain << phase;
            m_calibrationList << calibrationData;
        }
        m_calibrationLoaded = (m_calibrationList.count() > 0);
        emit calibrationLoadedChanged(m_calibrationLoaded);
        applyCalibration();

        setCalibration(calibration["enabled"].toBool());
    }
}
Measurement::Mode Measurement::mode() const
{
    return m_mode;
}
unsigned int Measurement::dataChanel() const
{
    return m_dataChanel;
}
void Measurement::setDataChanel(unsigned int channel)
{
    if (channel != m_dataChanel) {
        m_dataChanel = channel;
        emit dataChanelChanged(m_dataChanel);
    }
}
unsigned int Measurement::referenceChanel() const
{
    return m_referenceChanel;
}
void Measurement::setReferenceChanel(unsigned int channel)
{
    if (channel != m_referenceChanel) {
        m_referenceChanel = channel;
        emit referenceChanelChanged(m_referenceChanel);
    }
}
float Measurement::level() const
{
    return m_dataMeter.value();
}
float Measurement::referenceLevel() const
{
    return m_referenceMeter.value();
}
unsigned int Measurement::delay() const
{
    return m_delay;
}
void Measurement::setMode(const Measurement::Mode &mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        emit modeChanged(m_mode);
    }
}

void Measurement::setMode(QVariant mode)
{
    setMode(mode.value<Mode>());
}
QVariant Measurement::getAvailableModes() const
{
    QStringList typeList;
    for (const auto &type : m_modeMap) {
        typeList << type.second;
    }
    return typeList;
}
//this calls from timer thread
//should be called while mutex locked
void Measurement::updateFftPower()
{
    if (Q_LIKELY(m_mode == m_currentMode)) return;
    m_currentMode = m_mode;

    switch (m_currentMode) {
    case Mode::LFT:
        m_dataFT.setType(FourierTransform::Log);
        break;

    default:
        m_dataFT.setSize(pow(2, m_FFTsizes.at(m_currentMode)));
        m_dataFT.setType(FourierTransform::Fast);
    }
    m_dataFT.setSampleRate(sampleRate());
    m_dataFT.prepare();
    calculateDataLength();

    m_moduleAvg.setSize(size());
    m_magnitudeAvg.setSize(size());
    m_pahseAvg.setSize(size());
    m_coherence.setSize(size());

    m_moduleLPFs.resize(size());
    m_magnitudeLPFs.resize(size());
    m_phaseLPFs.resize(size());
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

        emit filtersFrequencyChanged(m_filtersFrequency);
    }
}
void Measurement::setFiltersFrequency(QVariant frequency)
{
    setFiltersFrequency(static_cast<Filter::Frequency>(frequency.toInt()));
}
Measurement::AverageType Measurement::averageType() const
{
    return m_averageType;
}
void Measurement::onSampleRateChanged()
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    m_sampleRate = m_audioStream->format().sampleRate;
    m_dataFT.setSampleRate(sampleRate());
    m_dataFT.prepare();
    calculateDataLength();
}
float Measurement::gain() const
{
    return 20 * std::log10(m_gain);
}
void Measurement::setGain(float gain)
{
    gain = std::pow(10, gain / 20.f);
    if (!qFuzzyCompare(m_gain, gain)) {
        m_gain = gain;
        emit gainChanged(m_gain);
    }
}
audio::DeviceInfo::Id Measurement::deviceId() const
{
    return m_deviceId;
}
void Measurement::setDeviceId(const audio::DeviceInfo::Id &deviceId)
{
    if (!deviceId.isNull() && deviceId != m_deviceId) {
        m_deviceId = deviceId;
        emit deviceIdChanged(m_deviceId);
        updateAudio();
    }
}
QString Measurement::deviceName() const
{
    return audio::Client::getInstance()->deviceName(m_deviceId);
}

void Measurement::selectDevice(const QString &name)
{
    auto id = audio::Client::getInstance()->deviceIdByName(name);
    setDeviceId(id);
}
void Measurement::calculateDataLength()
{
    auto frequencyList = m_dataFT.getFrequencies();
    m_dataLength = frequencyList.size();

    if (m_ftdata)
        delete[] m_ftdata;
    m_ftdata = new FTData[m_dataLength];
    unsigned int i = 0;
    for (auto frequency : frequencyList) {
        m_ftdata[i++].frequency = frequency;
    }
    applyCalibration();
}
void Measurement::setActive(bool active)
{
    if (active == m_active)
        return;
    std::lock_guard<std::mutex> guard(m_dataMutex);

    chart::Source::setActive(active);
    m_error = false;
    emit errorChanged(m_error);

    updateAudio();

    m_dataMeter.reset();
    m_referenceMeter.reset();
    m_loopBuffer.reset();
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measurement::setError()
{
    chart::Source::setActive(false);
    m_error = true;
    m_dataMeter.reset();
    m_referenceMeter.reset();
    m_input.close();
    emit errorChanged(m_error);
    emit levelChanged();
    emit referenceLevelChanged();
}

void Measurement::newSampleFromGenerator(float sample)
{
    if (m_loopBuffer.size() < 48000) {
        m_loopBuffer.push(sample);
    }
}

void Measurement::resetLoopBuffer()
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    m_loopBuffer.reset();
}
//this calls from gui thread
void Measurement::setDelay(unsigned int delay)
{
    m_delay = delay;
}

unsigned int Measurement::average() const
{
    return m_average;
}
//this calls from timer thread
void Measurement::updateDelay()
{
    if (m_resetDelay) {
        m_workingDelay = 0;
        m_reference.reset();
        m_data.reset();
        m_resetDelay = false;
    }
    if (m_workingDelay != m_delay) {
        long delta = static_cast<long>(m_workingDelay) - static_cast<long>(m_delay);
        m_workingDelay = m_delay;
        bool direction = std::signbit(static_cast<double>(delta));
        delta = std::abs(delta);
        for (long i = 0; i != delta; ++i) {
            if (direction) {
                m_reference.push(0.f);
            } else {
                m_data.push(0.f);
            }
        }
        emit delayChanged(m_workingDelay);
    }
}
void Measurement::setAverage(unsigned int average)
{
    if (m_average != average) {
        std::lock_guard<std::mutex> guard(m_dataMutex);
        m_average = average;
        m_deconvAvg.setDepth(m_average);
        m_moduleAvg.setDepth(m_average);
        m_magnitudeAvg.setDepth(m_average);
        m_pahseAvg.setDepth(m_average);
        emit averageChanged(m_average);
    }
}
bool Measurement::polarity() const
{
    return m_polarity;
}
void Measurement::setPolarity(bool polarity)
{
    if (m_polarity != polarity) {
        m_polarity = polarity;
        emit polarityChanged(m_polarity);
    }
}
Filter::Frequency Measurement::filtersFrequency() const
{
    return m_filtersFrequency;
}
void Measurement::setAverageType(AverageType type)
{
    if (m_averageType != type) {
        std::lock_guard<std::mutex> guard(m_dataMutex);
        m_averageType = type;
        emit averageTypeChanged(m_averageType);
    }
}
void Measurement::setAverageType(QVariant type)
{
    setAverageType(static_cast<AverageType>(type.toInt()));
}
unsigned int Measurement::sampleRate() const
{
    return m_sampleRate;
}
QVariant Measurement::getAvailableWindowTypes() const
{
    return WindowFunction::getTypes();
}
WindowFunction::Type Measurement::getWindowType() const
{
    return m_windowFunctionType;
}
void Measurement::setWindowType(WindowFunction::Type type)
{
    if (m_windowFunctionType != type) {
        m_windowFunctionType = type;
        m_dataFT.setWindowFunctionType(m_windowFunctionType);
        m_deconvolution.setWindowFunctionType(m_windowFunctionType);
        m_delayFinder.setWindowFunctionType(m_windowFunctionType);
        {
            std::lock_guard<std::mutex> guard(m_dataMutex);
            m_dataFT.prepare();
        }
        emit windowTypeChanged(m_windowFunctionType);
    }
}
void Measurement::setWindowType(QVariant type)
{
    setWindowType(static_cast<WindowFunction::Type>(type.toInt()));
}
void Measurement::writeData(const QByteArray &buffer)
{
    if (!m_audioStream) {
        return;
    }
    float sample;
    auto totalChanels = m_audioStream->format().channelCount;
    unsigned int currentChanel = 0;
    bool forceRef = referenceChanel() >= totalChanels;
    bool forceData = dataChanel() >= totalChanels;
    float loopSample = 0;

    std::lock_guard<std::mutex> guard(m_dataMutex);
    bool loopAvailable = m_loopBuffer.size() >= m_audioStream->depth() * buffer.size() / (totalChanels * sizeof(float));
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        if (currentChanel == 0) {
            loopSample = loopAvailable ? m_loopBuffer.pop() : 0;
        }

        if (currentChanel == dataChanel()) {
            memcpy(&sample, it, sizeof(float));
            m_data.pushnpop((m_polarity ? -1 * sample : sample), 48000);
            m_dataMeter.add(sample);
        }

        if (currentChanel == referenceChanel()) {
            memcpy(&sample, it, sizeof(float));
            m_reference.pushnpop(sample, 48000);
            m_referenceMeter.add(sample);
        }
        ++currentChanel;
        if (currentChanel >= totalChanels) {
            if (forceRef) {
                m_reference.pushnpop(loopSample, 48000);
                m_referenceMeter.add(loopSample);
            }
            if (forceData) {
                m_data.pushnpop(loopSample, 48000);
                m_dataMeter.add(loopSample);
            }
            currentChanel = 0;
        }
        it += 3;
    }

}
void Measurement::transform()
{
    if (!m_active || m_error)
        return;

    lock();
    updateFftPower();
    updateDelay();

    float d, r;
    while (m_data.size() > 0 && m_reference.size() > 0) {
        d = m_data.pop() * m_gain;
        r = m_reference.pop();

        m_dataFT.add(d, r);
        m_deconvolution.add(r, d);
        m_delayFinder.add(r, d);
    }
    m_dataFT.transform(true);
    m_deconvolution.transform();
    if ((++m_delayFinderCounter % 25) == 0) {
        m_delayFinder.transform();
        m_delayFinderCounter = 0;
    }
    averaging();
    unlock();
    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measurement::averaging()
{
    complex p;
    int j;
    for (unsigned int i = 0; i < m_dataLength ; i++) {

        j = static_cast<int>(i);
        float calibratedA = m_dataFT.af(i).abs();
        //TODO: think and do
        //if (calibratedA < someThresholdInDb ) continue;

        if (m_enableCalibration && m_calibrationGain.size() > j) {
            calibratedA /= m_calibrationGain[j];
            p.polar(m_dataFT.bf(i).arg() - m_dataFT.af(i).arg() - m_calibrationPhase[j]);
        } else {
            p.polar(m_dataFT.bf(i).arg() - m_dataFT.af(i).arg());
        }

        float magnitude = calibratedA / m_dataFT.bf(i).abs();
#ifdef WIN64
        if (magnitude / 0.f == magnitude) {
#else
        if (std::isnan(magnitude) || std::isinf(magnitude)) {
#endif
            magnitude = 0.f;
        }

        switch (averageType()) {
        case AverageType::Off:
            m_ftdata[i].magnitude = magnitude;
            m_ftdata[i].module    = calibratedA;
            m_ftdata[i].phase     = p;
            break;

        case AverageType::LPF:
            m_ftdata[i].magnitude = m_magnitudeLPFs[i](magnitude);
            m_ftdata[i].module    = m_moduleLPFs[i](calibratedA);
            m_ftdata[i].phase     = m_phaseLPFs[i](p);
            break;

        case AverageType::FIFO:
            m_magnitudeAvg.append(i, magnitude );
            m_moduleAvg.append(i,    calibratedA );
            m_pahseAvg.append(i,     p);

            m_ftdata[i].magnitude = m_magnitudeAvg.value(i);
            m_ftdata[i].module    = m_moduleAvg.value(i);
            m_ftdata[i].phase     = m_pahseAvg.value(i);
            break;
        }

        m_coherence.append(i, m_dataFT.bf(i), m_dataFT.af(i));
        m_ftdata[i].coherence = m_coherence.value(i);
    }

    int t = 0;
    float kt = 1000.f / sampleRate();
    for (unsigned int i = 0, j = m_deconvolutionSize / 2 - 1; i < m_deconvolutionSize; i++, j++, t++) {

        if (t > static_cast<int>(m_deconvolutionSize / 2)) {
            t -= static_cast<int>(m_deconvolutionSize);
            j -= m_deconvolutionSize;
        }

        switch (averageType()) {
        case AverageType::Off:
            m_impulseData[j].value.real = m_deconvolution.get(i);
            break;
        case AverageType::LPF:
            m_impulseData[j].value.real = m_deconvLPFs[i](m_deconvolution.get(i));
            break;
        case AverageType::FIFO:
            m_deconvAvg.append(i, m_deconvolution.get(i));
            m_impulseData[j].value.real = m_deconvAvg.value(i);
            break;
        }
        m_impulseData[j].time  = t * kt;//ms
    }
    m_estimatedDelay = m_delayFinder.maxIndex();
    emit estimatedChanged();
}
QObject *Measurement::store()
{
    auto *store = new Stored();
    store->build(this);

    QString avg;
    switch (m_averageType) {
    case Off:
        avg = "none";
        break;
    case LPF:
        avg = "LPF ";
        switch (m_filtersFrequency) {
        case Filter::Frequency::FourthHz:
            ;
            avg += "0.25 Hz";
            break;
        case Filter::Frequency::HalfHz:
            ;
            avg += "0.5 Hz";
            break;
        case Filter::Frequency::OneHz:
            ;
            avg += "1 Hz";
            break;
        }
        break;
    case FIFO:
        avg = "FIFO " + QString::number(m_average);
        break;
    }
    QString modeNote;
    switch (mode()) {
    case LFT:
        modeNote = "FT log time window";
        break;
    default:
        modeNote = "FFT power " + m_modeMap.at(mode());
    }

    store->setNotes(
        modeNote + "\t" +
        "delay: " + QString("%1").arg(1000.0 * delay() / sampleRate(), 0, 'f', 2) + "ms " +
        "gain: " + QString("%1").arg(gain(), 0, 'f', 2) + "dB \n" +
        deviceName() + QString(" R: %1").arg(referenceChanel() + 1) +
        QString(" M: %1").arg(dataChanel() + 1) +
        (polarity() ? " polarity inversed" : "") +
        (calibration() ? " calibrated" : "") + " \n" +
        "Window: " + WindowFunction::name(m_windowFunctionType) + "\t"
        "Average: " + avg + "\n"
        "Date: " + QDateTime::currentDateTime().toString()

    );

    return store;
}
chart::Source *Measurement::clone() const
{
    auto cloned = new Measurement(nullptr, parent());
    cloned->setActive(false);
    cloned->setMode(mode());
    cloned->setAverageType(averageType());
    cloned->setAverage(average());
    cloned->setFiltersFrequency(filtersFrequency());
    cloned->setPolarity(polarity());
    cloned->setDataChanel(dataChanel());
    cloned->setReferenceChanel(referenceChanel());
    cloned->setWindowType(getWindowType());

    cloned->setCalibration(calibration());
    cloned->m_calibrationList = m_calibrationList;
    cloned->m_calibrationLoaded = m_calibrationLoaded;
    cloned->applyCalibration();

    cloned->setDelay(delay());
    cloned->setGain(gain());
    cloned->setDeviceId(deviceId());

    cloned->setName(name());
    cloned->setActive(active());
    return cloned;
}
long Measurement::estimated() const noexcept
{
    if (m_estimatedDelay > m_delayFinder.size() / 2) {
        return m_estimatedDelay - m_delayFinder.size() + static_cast<long>(m_workingDelay);
    }
    return m_estimatedDelay + static_cast<long>(m_workingDelay);
}

long Measurement::estimatedDelta() const noexcept
{
    if (m_estimatedDelay > m_delayFinder.size() / 2) {
        return m_estimatedDelay - m_delayFinder.size();
    }
    return m_estimatedDelay;
}
bool Measurement::calibration() const noexcept
{
    return m_enableCalibration;
}
bool Measurement::calibrationLoaded() const noexcept
{
    return m_calibrationLoaded;
}
void Measurement::setCalibration(bool c) noexcept
{
    if (c != m_enableCalibration) {
        m_enableCalibration = c;
        emit calibrationChanged(m_enableCalibration);
    }
}
bool Measurement::loadCalibrationFile(const QUrl &fileName) noexcept
{
    QFile loadFile(fileName.toLocalFile());
    if (!loadFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open file");
        return false;
    }

    QTextStream in(&loadFile);
    m_calibrationList.clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.at(0).isDigit())
            continue;

        QStringList row = line.split("\t");
        float frequency = 0.f, gain = 0.f, phase = 0.f;
        if (row.size() > 0) frequency   = row[0].toFloat();

        if (row.size() > 1) gain        = row[1].toFloat();
        else continue;

        if (row.size() > 2) phase       = row[2].toFloat();

        QVector<float> calibrationData;
        calibrationData << frequency << gain << phase;
        m_calibrationList << calibrationData;
    }
    loadFile.close();

    m_calibrationLoaded = (m_calibrationList.count() > 0);
    emit calibrationLoadedChanged(m_calibrationLoaded);
    applyCalibration();
    return m_calibrationLoaded;
}
void Measurement::applyCalibration()
{
    if (!m_calibrationLoaded || !m_calibrationList.size())
        return;

    m_calibrationGain.resize(static_cast<int>(m_dataLength));
    m_calibrationPhase.resize(static_cast<int>(m_dataLength));

    QVector<float> last = m_calibrationList[0];
    last[0] = 0.f;

    int j = 0;
    float
    kg, bg, kp, bp,
    g1, g2, f1, f2, p1, p2,
    g, p;
    bool inList = false;
    for (int i = 0; i < static_cast<int>(m_dataLength); ++i) {

        while (m_ftdata[i].frequency > m_calibrationList[j][0]) {
            last = m_calibrationList[j];
            if (j + 1 < m_calibrationList.size()) {
                ++j;
                inList = true;
            } else {
                inList = false;
                break;
            }
        }

        f1 = last[0];
        g1 = last[1];
        p1 = last[2];
        f2 = m_calibrationList[j][0];
        g2 = m_calibrationList[j][1];
        p2 = m_calibrationList[j][2];

        if (inList) {
            kg = (g2 - g1) / (f2 - f1);
            bg = g2 - f2 * kg;

            kp = (p2 - p1) / (f2 - f1);
            bp = p2 - f2 * kp;

            g = kg * m_ftdata[i].frequency + bg;
            p = kp * m_ftdata[i].frequency + bp;
        } else {
            g = m_calibrationList[j][1];
            p = m_calibrationList[j][2];
        }

        m_calibrationGain[i] = pow(10.f, 0.05f * g);
        m_calibrationPhase[i] = p * static_cast<float>(M_PI / 180.0);
    }
}
void Measurement::updateAudio()
{
    if (m_audioStream) {
        m_input.close();
        m_audioStream->close();
    }
    m_audioStream = nullptr;
    checkChannels();
    if (m_active) {
        std::async([this]() {
            audio::Format format = audio::Client::getInstance()->deviceInputFormat(m_deviceId);
            m_sampleRate = format.sampleRate;
            m_input.setCallback([this](const QByteArray & buffer) {
                writeData(buffer);
            });
            m_audioStream = audio::Client::getInstance()->openInput(m_deviceId, &m_input, format);
            connect(m_audioStream, &audio::Stream::sampleRateChanged, this, &Measurement::onSampleRateChanged);
            if (!m_audioStream) {
                setError();
                return;
            }
            emit audioFormatChanged();
        });
    }
}

void Measurement::checkChannels()
{
    audio::Format format = audio::Client::getInstance()->deviceInputFormat(m_deviceId);
    if (m_referenceChanel > format.channelCount) {
        setReferenceChanel(format.channelCount - 1);
    }
    if (m_dataChanel > format.channelCount) {
        setDataChanel(format.channelCount - 1);
    }
}
void Measurement::resetAverage() noexcept
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    m_deconvAvg.reset();
    m_moduleAvg.reset();
    m_magnitudeAvg.reset();
    m_pahseAvg.reset();

    auto reset = [](auto * f) {
        f->reset();
    };
    m_moduleLPFs.each(reset);
    m_magnitudeLPFs.each(reset);
    m_deconvLPFs.each(reset);
    m_phaseLPFs.each(reset);
    m_loopBuffer.reset();
}
