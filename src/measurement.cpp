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
#include <QFile>
#include <QUrl>
#include <QJsonArray>
#include <QDateTime>
#include <QtMath>
#include <algorithm>
#include "measurement.h"

Measurement::Measurement(Settings *settings, QObject *parent) : Fftchart::Source(parent),
    m_timer(nullptr), m_timerThread(nullptr),
    m_audioThread(nullptr),
    m_settings(settings),
    m_mode(FFT14), m_currentMode(),
    m_average(1),
    m_delay(0), m_setDelay(0), m_gain(1.f),
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

    connect(&m_audioThread, SIGNAL(recived(const QByteArray &)), SLOT(writeData(const QByteArray &)),
            Qt::DirectConnection);
    connect(&m_audioThread, SIGNAL(deviceChanged(QString)), this, SIGNAL(deviceChanged(QString)));
    connect(&m_audioThread, SIGNAL(deviceChanged(QString)), this, SIGNAL(chanelsCountChanged()));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SIGNAL(chanelsCountChanged()));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SLOT(recalculateDataLength()));
    connect(&m_audioThread, SIGNAL(formatChanged()), this, SIGNAL(sampleRateChanged()));
    connect(&m_audioThread, SIGNAL(deviceError()), this, SLOT(setError()));

    QAudioDeviceInfo device(QAudioDeviceInfo::defaultInputDevice());
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
                                                                                &Measurement::gainChanged,          delay()).toFloat());
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
        if (!selectDevice(   m_settings->reactValue<Measurement, QString>(           "device",       this,
                                                                                     &Measurement::deviceChanged,        device.deviceName()).toString())) {
            selectDevice(device);
        }
    } else {
        selectDevice(device);
    }
    m_deconvolutionSize = static_cast<unsigned int>(pow(2, 12));

    calculateDataLength();
    m_dataFT.prepare();
    m_moduleLPFs.resize(m_dataLength);
    m_magnitudeLPFs.resize(m_dataLength);
    m_phaseLPFs.resize(m_dataLength);

    m_deconvolution.setSize(m_deconvolutionSize);
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
    m_timerThread.start();
    setActive(true);

    modeMap = {
        {FFT10, "10"},
        {FFT12, "12"},
        {FFT14, "14"},
        {FFT15, "15"},
        {FFT16, "16"},
        {LFT,   "LTW"}
    };
    FFTsizes = {
        {FFT10, 10},
        {FFT12, 12},
        {FFT14, 14},
        {FFT15, 15},
        {FFT16, 16}
    };
}
Measurement::~Measurement()
{
    m_audioThread.quit();
    m_audioThread.wait();

    m_timerThread.quit();
    m_timerThread.wait();
}
QJsonObject Measurement::toJSON() const noexcept
{
    QJsonObject data;
    data["active"]          = active();
    data["name"]            = name();
    data["delay"]           = static_cast<int>(delay());
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

void Measurement::fromJSON(QJsonObject data) noexcept
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

    foreach (const QAudioDeviceInfo &deviceInfo,
             QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
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
    foreach (const QAudioDeviceInfo &deviceInfo,
             QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
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
void Measurement::setMode(const Measurement::Mode &mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        emit modeChanged(m_mode);
    }
}
QVariant Measurement::getAvailableModes() const
{
    QStringList typeList;
    for (const auto &type : modeMap) {
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
        m_dataFT.setSize(pow(2, FFTsizes[m_currentMode]));
        m_dataFT.setType(FourierTransform::Fast);
    }
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
void Measurement::recalculateDataLength()
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
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

void Measurement::calculateDataLength()
{
    auto frequencyList = m_dataFT.getFrequencies(sampleRate());
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

    Fftchart::Source::setActive(active);
    m_error = false;
    emit errorChanged(m_error);

    QMetaObject::invokeMethod(
        &m_audioThread,
        "setActive",
        Qt::QueuedConnection,
        Q_ARG(bool, active)
    );

    m_dataMeter.reset();
    m_referenceMeter.reset();
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measurement::setError()
{
    Fftchart::Source::setActive(false);
    m_error = true;
    m_dataMeter.reset();
    m_referenceMeter.reset();
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
        bool direction = std::signbit(static_cast<double>(delta));
        delta = std::abs(delta);
        for (long i = 0; i != delta; ++i) {
            if (direction) {
                m_reference.push(0.f);
            } else {
                m_data.push(0.f);
            }
        }
        emit delayChanged(m_delay);
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
        std::lock_guard<std::mutex> guard(m_dataMutex);
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
    if (m_windowFunctionType != type) {
        m_windowFunctionType = type;
        m_dataFT.setWindowFunctionType(m_windowFunctionType);
        m_deconvolution.setWindowFunctionType(m_windowFunctionType);
        {
            std::lock_guard<std::mutex> guard(m_dataMutex);
            m_dataFT.prepare();
        }
        emit windowTypeChanged(m_windowFunctionType);
    }
}
void Measurement::writeData(const QByteArray &buffer)
{
    float sample;
    auto totalChanels = static_cast<unsigned int>(m_audioThread.format().channelCount());
    unsigned int currentChanel = 0;
    std::lock_guard<std::mutex> guard(m_dataMutex);
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {

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
    }
    m_dataFT.transform(true);
    m_deconvolution.transform();
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
    float max(0.f);
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

        if (max < abs(m_impulseData[j].value.real)) {
            max = abs(m_impulseData[j].value.real);
            m_estimatedDelay = i;
        }
        m_impulseData[j].time  = t * kt;//ms
    }
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
        modeNote = "FFT power " + modeMap[mode()];
    }

    store->setNotes(
        modeNote + "\t" +
        "delay: " + QString("%1").arg(1000.0 * delay() / sampleRate(), 0, 'f', 2) + "ms \t" +
        (polarity() ? "polarity inversed" : "") + "\n"
        "Window: " + WindowFunction::name(m_windowFunctionType) + "\t"
        "Average: " + avg + "\n"
        "Date: " + QDateTime::currentDateTime().toString()

    );

    return store;
}
long Measurement::estimated() const noexcept
{
    if (m_estimatedDelay > m_deconvolutionSize / 2) {
        return m_estimatedDelay - m_deconvolutionSize + static_cast<long>(m_delay);
    }
    return m_estimatedDelay + static_cast<long>(m_delay);
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
}
