/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "firsource.h"
#include "sourcelist.h"
#include "math/firdesign.h"
#include <QFile>
#include <QTextStream>
#include <QtMath>
#include <cmath>
#include <algorithm>
#include <iterator>

namespace {

// Smoothing denominator for each index in smoothingValues
constexpr int kSmoothingN[] = { 0, 1, 3, 6, 12, 24 };

// Sample rate of the source data. Impulse times are in ms, so 1 / dt is the rate
// in kHz, rounded to 100 Hz. The source must be locked.
unsigned int sourceSampleRate(const Abstract::Source &source)
{
    if (source.timeDomainSize() > 2) {
        const float dt = std::abs(source.impulseTime(1) - source.impulseTime(0));
        if (dt > 0.f) {
            return static_cast<unsigned int>(std::round(10.f / dt)) * 100u;
        }
    }
    return source.sampleRate();
}

// Time of the impulse response peak in seconds, refined by parabolic interpolation.
// The source must be locked.
float impulsePeak(const Abstract::Source &source)
{
    const unsigned int size = source.timeDomainSize();
    if (size < 3) {
        return 0.f;
    }

    unsigned int peak = 0;
    float peakValue = 0.f;
    for (unsigned int i = 0; i < size; ++i) {
        const float value = std::abs(source.impulseValue(i));
        if (value > peakValue) {
            peakValue = value;
            peak = i;
        }
    }

    float time = source.impulseTime(peak);
    if (peak > 0 && peak + 1 < size) {
        const float before = std::abs(source.impulseValue(peak - 1));
        const float after  = std::abs(source.impulseValue(peak + 1));
        const float curve  = before - 2.f * peakValue + after;
        if (curve < 0.f) {
            const float shift = 0.5f * (before - after) / curve;
            time += shift * (source.impulseTime(peak + 1) - source.impulseTime(peak));
        }
    }
    return time / 1000.f;
}

} // namespace

FirSource::FirSource(QObject *parent)
    : Abstract::Source(parent),
      m_source(nullptr),
      m_smoothing(0),
      m_highpass(20.f),
      m_highpassEnabled(true),
      m_lowpass(20000.f),
      m_lowpassEnabled(true),
      m_tapCount(512),
      m_transition(3),   // Firm fade by default
      m_targetSampleRate(0),
      m_phaseMode(FirDesign::LinearPhase),
      m_maxBoost(6.f),
      m_minFrequency(0.f),
      m_latency(0.f)
{
    setObjectName("FirSource");
    setName("FIR Export");

    connect(this, &FirSource::sourceChanged,           this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::smoothingChanged,        this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::highpassChanged,         this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::lowpassChanged,          this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::tapCountChanged,         this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::transitionChanged,       this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::highpassEnabledChanged,  this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::lowpassEnabledChanged,   this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::targetSampleRateChanged, this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::phaseModeChanged,        this, &FirSource::scheduleUpdate);
    connect(this, &FirSource::maxBoostChanged,         this, &FirSource::scheduleUpdate);
}

Shared::Source FirSource::clone() const
{
    auto cloned = std::make_shared<FirSource>(parent());
    cloned->setSource(source());
    cloned->setSmoothing(m_smoothing);
    cloned->setHighpass(m_highpass);
    cloned->setLowpass(m_lowpass);
    cloned->setTapCount(m_tapCount);
    cloned->setTransition(m_transition);
    cloned->setHighpassEnabled(m_highpassEnabled);
    cloned->setLowpassEnabled(m_lowpassEnabled);
    cloned->setTargetSampleRate(m_targetSampleRate);
    cloned->setPhaseMode(m_phaseMode);
    cloned->setMaxBoost(m_maxBoost);
    cloned->setName(name());
    cloned->setActive(active());
    cloned->update();
    return std::static_pointer_cast<Abstract::Source>(cloned);
}

QJsonObject FirSource::toJSON() const noexcept
{
    auto obj = Abstract::Source::toJSON();
    obj["smoothing"]        = m_smoothing;
    obj["highpass"]         = static_cast<double>(m_highpass);
    obj["lowpass"]          = static_cast<double>(m_lowpass);
    obj["tapCount"]         = m_tapCount;
    obj["transition"]       = m_transition;
    obj["highpassEnabled"]  = m_highpassEnabled;
    obj["lowpassEnabled"]   = m_lowpassEnabled;
    obj["targetSampleRate"] = m_targetSampleRate;
    obj["phaseMode"]        = m_phaseMode;
    obj["maxBoost"]         = static_cast<double>(m_maxBoost);
    if (m_source) {
        obj["sourceId"] = m_source->uuid().toString();
    }
    return obj;
}

void FirSource::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    Abstract::Source::fromJSON(data);
    setSmoothing(       data["smoothing"       ].toInt(0));
    setHighpass(        static_cast<float>(data["highpass"].toDouble(20.0)));
    setLowpass(         static_cast<float>(data["lowpass" ].toDouble(20000.0)));
    setTapCount(        data["tapCount"        ].toInt(512));
    setTransition(      data["transition"      ].toInt(3));
    setHighpassEnabled( data["highpassEnabled" ].toBool(true));
    setLowpassEnabled(  data["lowpassEnabled"  ].toBool(true));
    setTargetSampleRate(data["targetSampleRate"].toInt(0));
    setPhaseMode(       data["phaseMode"       ].toInt(FirDesign::LinearPhase));
    setMaxBoost(        static_cast<float>(data["maxBoost"].toDouble(6.0)));

    if (list && data.contains("sourceId")) {
        auto id = QUuid(data["sourceId"].toString());
        setSource(list->getByUUid(id));
    }
}

Shared::Source FirSource::source() const { return m_source; }

void FirSource::setSource(const Shared::Source &src)
{
    if (m_source == src || src.get() == this) return;

    if (m_source) {
        disconnect(m_source.get(), &Abstract::Source::readyRead, this, &FirSource::scheduleUpdate);
    }
    m_source = src;
    if (m_source) {
        // readyRead may come from the measurement thread: handle it in our own thread
        connect(m_source.get(), &Abstract::Source::readyRead,
                this, &FirSource::scheduleUpdate, Qt::QueuedConnection);
    }
    emit sourceChanged();
}

QUuid FirSource::sourceId() const
{
    return m_source ? m_source->uuid() : QUuid{};
}

int FirSource::smoothing() const { return m_smoothing; }
void FirSource::setSmoothing(int v)
{
    v = std::clamp(v, 0, static_cast<int>(std::size(kSmoothingN)) - 1);
    if (m_smoothing != v) { m_smoothing = v; emit smoothingChanged(); }
}

float FirSource::highpass() const { return m_highpass; }
void FirSource::setHighpass(float v)
{
    v = std::max(v, 0.f);
    if (!qFuzzyCompare(m_highpass, v)) { m_highpass = v; emit highpassChanged(); }
}

float FirSource::lowpass() const { return m_lowpass; }
void FirSource::setLowpass(float v)
{
    v = std::max(v, 0.f);
    if (!qFuzzyCompare(m_lowpass, v)) { m_lowpass = v; emit lowpassChanged(); }
}

int FirSource::tapCount() const { return m_tapCount; }
void FirSource::setTapCount(int v)
{
    v = std::clamp(v, 16, 16384);
    if (m_tapCount != v) { m_tapCount = v; emit tapCountChanged(); }
}

int FirSource::transition() const { return m_transition; }
void FirSource::setTransition(int v)
{
    v = std::clamp(v, 1, 16);
    if (m_transition != v) { m_transition = v; emit transitionChanged(); }
}

bool FirSource::highpassEnabled() const { return m_highpassEnabled; }
void FirSource::setHighpassEnabled(bool v) { if (m_highpassEnabled != v) { m_highpassEnabled = v; emit highpassEnabledChanged(); } }

bool FirSource::lowpassEnabled() const { return m_lowpassEnabled; }
void FirSource::setLowpassEnabled(bool v) { if (m_lowpassEnabled != v) { m_lowpassEnabled = v; emit lowpassEnabledChanged(); } }

int FirSource::targetSampleRate() const { return m_targetSampleRate; }
void FirSource::setTargetSampleRate(int v)
{
    v = std::max(v, 0);
    if (m_targetSampleRate != v) { m_targetSampleRate = v; emit targetSampleRateChanged(); }
}

int FirSource::phaseMode() const { return m_phaseMode; }
void FirSource::setPhaseMode(int v)
{
    v = std::clamp(v, static_cast<int>(FirDesign::LinearPhase), static_cast<int>(FirDesign::FullPhase));
    if (m_phaseMode != v) { m_phaseMode = v; emit phaseModeChanged(); }
}

float FirSource::maxBoost() const { return m_maxBoost; }
void FirSource::setMaxBoost(float v)
{
    v = std::clamp(v, 0.f, 24.f);
    if (!qFuzzyCompare(m_maxBoost, v)) { m_maxBoost = v; emit maxBoostChanged(); }
}

float FirSource::minFrequency() const { return m_minFrequency; }

float FirSource::latency() const { return m_latency; }

bool FirSource::ready() const
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    return !m_taps.empty();
}

QVariant FirSource::sampleRateValues() const
{
    return QVariant::fromValue(QStringList{"Auto", "44100 Hz", "48000 Hz", "88200 Hz", "96000 Hz", "192000 Hz"});
}

QVariant FirSource::smoothingValues() const
{
    return QVariant::fromValue(QStringList{"Off", "1 oct", "1/3 oct", "1/6 oct", "1/12 oct", "1/24 oct"});
}

void FirSource::scheduleUpdate()
{
    if (m_updateScheduled) {
        return;
    }
    m_updateScheduled = true;
    QMetaObject::invokeMethod(this, [this]() {
        if (m_updateScheduled) {
            update();
        }
    }, Qt::QueuedConnection);
}

// -----------------------------------------------------------------------
void FirSource::update()
{
    m_updateScheduled = false;
    const bool wasReady = ready();

    // ── Snapshot the source (source locked, own data untouched) ─────────────
    FirDesign::Input input;
    unsigned int sampleRate = m_targetSampleRate > 0 ? static_cast<unsigned int>(m_targetSampleRate) : 48000u;

    if (m_source) {
        m_source->lock();
        if (m_targetSampleRate <= 0) {
            sampleRate = sourceSampleRate(*m_source);
        }
        const float nyquist = static_cast<float>(sampleRate) / 2.f;
        const unsigned int size = m_source->frequencyDomainSize();
        input.frequency.reserve(size);
        input.magnitude.reserve(size);
        input.phase.reserve(size);
        input.coherence.reserve(size);
        for (unsigned int i = 0; i < size; ++i) {
            const float f = m_source->frequency(i);
            if (f <= 0.f || f > nyquist) {
                continue;
            }
            input.frequency.push_back(f);
            input.magnitude.push_back(m_source->magnitudeRaw(i));
            input.phase.push_back(m_source->phase(i));
            input.coherence.push_back(m_source->coherence(i));
        }
        if (m_phaseMode == FirDesign::FullPhase) {
            input.delay = impulsePeak(*m_source);
        }
        m_source->unlock();
    }

    // ── Design ──────────────────────────────────────────────────────────────
    FirDesign::Parameters parameters;
    parameters.sampleRate = sampleRate;
    parameters.taps       = static_cast<unsigned int>(m_tapCount);
    parameters.phaseMode  = static_cast<FirDesign::PhaseMode>(m_phaseMode);
    parameters.smoothing  = kSmoothingN[m_smoothing];
    parameters.lowLimit   = m_highpassEnabled ? m_highpass : 0.f;
    parameters.highLimit  = m_lowpassEnabled  ? m_lowpass  : 0.f;
    parameters.transition = static_cast<float>(m_transition);
    parameters.maxBoost   = m_maxBoost;

    auto result = FirDesign::design(input, parameters);

    // ── Publish (own data locked) ───────────────────────────────────────────
    {
        std::lock_guard<std::mutex> guard(m_dataMutex);

        setFrequencyDomainSize(static_cast<unsigned int>(result.response.size()));
        for (size_t j = 0; j < result.response.size(); ++j) {
            const float abs = result.response[j].abs();
            m_ftdata[j]           = {};
            m_ftdata[j].frequency = input.frequency[j];
            m_ftdata[j].module    = abs;
            m_ftdata[j].magnitude = abs;
            m_ftdata[j].phase     = abs > 0.f ? result.response[j] / abs : Complex{1.f, 0.f};
            m_ftdata[j].coherence = 1.f;
        }

        const float kt = 1000.f / static_cast<float>(sampleRate);  // ms per sample
        setTimeDomainSize(static_cast<unsigned int>(result.taps.size()));
        for (size_t i = 0; i < result.taps.size(); ++i) {
            m_impulseData[i].time  = (static_cast<int>(i) - static_cast<int>(result.latency)) * kt;
            m_impulseData[i].value = result.taps[i];
        }

        m_taps = std::move(result.taps);
    }

    setSampleRate(sampleRate);

    const float minFrequency = FirDesign::minFrequency(sampleRate, parameters.taps);
    if (!qFuzzyCompare(m_minFrequency, minFrequency)) {
        m_minFrequency = minFrequency;
        emit minFrequencyChanged();
    }
    const float latency = 1000.f * static_cast<float>(result.latency) / static_cast<float>(sampleRate);
    if (!qFuzzyCompare(m_latency, latency)) {
        m_latency = latency;
        emit latencyChanged();
    }
    if (wasReady != ready()) {
        emit readyChanged();
    }
    emit readyRead();
}

// -----------------------------------------------------------------------
bool FirSource::exportFIR(const QUrl &fileName) const noexcept
{
    std::vector<float> taps;
    {
        std::lock_guard<std::mutex> guard(m_dataMutex);
        taps = m_taps;
    }
    if (taps.empty()) {
        return false;
    }

    QFile file(fileName.toLocalFile());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("FirSource: couldn't open export file.");
        return false;
    }
    QTextStream out(&file);
    out.setRealNumberPrecision(10);
    for (auto tap : taps) {
        out << tap << "\n";
    }
    file.close();
    return true;
}
