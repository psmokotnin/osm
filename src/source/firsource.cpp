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
#include <QFile>
#include <QTextStream>
#include <QtMath>
#include <cmath>
#include <algorithm>

FirSource::FirSource(QObject *parent)
    : Abstract::Source(parent),
      m_source(nullptr),
      m_smoothing(0),
      m_highpass(0.f),
      m_lowpass(20000.f),
      m_tapCount(512),
      m_slope(24.f),
      m_minFrequency(0.f)
{
    setObjectName("FirSource");
    setName("FIR Export");

    connect(this, &FirSource::sourceChanged,   this, &FirSource::update);
    connect(this, &FirSource::smoothingChanged, this, &FirSource::update);
    connect(this, &FirSource::highpassChanged,  this, &FirSource::update);
    connect(this, &FirSource::lowpassChanged,   this, &FirSource::update);
    connect(this, &FirSource::tapCountChanged,  this, &FirSource::update);
    connect(this, &FirSource::slopeChanged,     this, &FirSource::update);
}

Shared::Source FirSource::clone() const
{
    auto cloned = std::make_shared<FirSource>(parent());
    cloned->setSource(source());
    cloned->setSmoothing(m_smoothing);
    cloned->setHighpass(m_highpass);
    cloned->setLowpass(m_lowpass);
    cloned->setTapCount(m_tapCount);
    cloned->setSlope(m_slope);
    cloned->setName(name());
    cloned->setActive(active());
    return std::static_pointer_cast<Abstract::Source>(cloned);
}

QJsonObject FirSource::toJSON() const noexcept
{
    auto obj = Abstract::Source::toJSON();
    obj["smoothing"] = m_smoothing;
    obj["highpass"]  = static_cast<double>(m_highpass);
    obj["lowpass"]   = static_cast<double>(m_lowpass);
    obj["tapCount"]  = m_tapCount;
    obj["slope"]     = static_cast<double>(m_slope);
    if (m_source) {
        obj["sourceId"] = m_source->uuid().toString();
    }
    return obj;
}

void FirSource::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    Abstract::Source::fromJSON(data);
    setSmoothing(data["smoothing"].toInt(0));
    setHighpass(static_cast<float>(data["highpass"].toDouble(0.0)));
    setLowpass( static_cast<float>(data["lowpass" ].toDouble(20000.0)));
    setTapCount(data["tapCount"].toInt(512));
    setSlope(   static_cast<float>(data["slope"   ].toDouble(24.0)));

    if (list && data.contains("sourceId")) {
        auto id = QUuid(data["sourceId"].toString());
        setSource(list->getByUUid(id));
    }
}

Shared::Source FirSource::source() const { return m_source; }

void FirSource::setSource(const Shared::Source &src)
{
    if (m_source == src) return;

    if (m_source) {
        disconnect(m_source.get(), &Abstract::Source::readyRead, this, &FirSource::update);
    }
    m_source = src;
    if (m_source) {
        connect(m_source.get(), &Abstract::Source::readyRead, this, &FirSource::update);
    }
    emit sourceChanged();
}

QUuid FirSource::sourceId() const
{
    return m_source ? m_source->uuid() : QUuid{};
}

int FirSource::smoothing() const { return m_smoothing; }
void FirSource::setSmoothing(int v) { if (m_smoothing != v) { m_smoothing = v; emit smoothingChanged(); } }

float FirSource::highpass() const { return m_highpass; }
void FirSource::setHighpass(float v) { if (!qFuzzyCompare(m_highpass, v)) { m_highpass = v; emit highpassChanged(); } }

float FirSource::lowpass() const { return m_lowpass; }
void FirSource::setLowpass(float v) { if (!qFuzzyCompare(m_lowpass, v)) { m_lowpass = v; emit lowpassChanged(); } }

int FirSource::tapCount() const { return m_tapCount; }
void FirSource::setTapCount(int v) { if (m_tapCount != v) { m_tapCount = v; emit tapCountChanged(); } }

float FirSource::slope() const { return m_slope; }
void FirSource::setSlope(float v) { if (!qFuzzyCompare(m_slope, v)) { m_slope = v; emit slopeChanged(); } }

float FirSource::minFrequency() const { return m_minFrequency; }

QVariant FirSource::smoothingValues() const
{
    return QVariant::fromValue(QStringList{"Off", "1 oct", "1/3 oct", "1/6 oct", "1/12 oct", "1/24 oct"});
}

// Smoothing denominator for each index in smoothingValues
static const int s_smoothingN[] = { 0, 1, 3, 6, 12, 24 };

// -----------------------------------------------------------------------
void FirSource::update()
{
    if (!m_source || m_source->frequencyDomainSize() == 0) {
        setFrequencyDomainSize(0);
        emit readyRead();
        return;
    }

    m_source->lock();
    const unsigned int srcSize = m_source->frequencyDomainSize();

    // Infer sample rate from impulse time steps (same method as Windowing)
    unsigned int sr = 48000;
    if (m_source->timeDomainSize() > 2) {
        float dt = std::abs(m_source->impulseTime(1) - m_source->impulseTime(0));
        if (dt > 0.f)
            sr = static_cast<unsigned int>(std::round(10.f / dt)) * 100u;
    }

    // Frequency range representable by the selected tap count:
    //   f_min = sr / tapCount  (first non-DC bin)
    //   f_max = sr / 2         (Nyquist)
    const float fMin = static_cast<float>(sr) / static_cast<float>(m_tapCount);
    const float fMax = static_cast<float>(sr) / 2.f;

    // Collect only source bins that fall within [fMin, fMax]
    std::vector<unsigned int> validIdx;
    validIdx.reserve(srcSize);
    for (unsigned int i = 0; i < srcSize; ++i) {
        float f = m_source->frequency(i);
        if (f >= fMin && f <= fMax)
            validIdx.push_back(i);
    }

    const unsigned int M = static_cast<unsigned int>(validIdx.size());
    setFrequencyDomainSize(M);

    // Step 1 — read magnitudes from source (floor at -40 dB prevents infinite boost).
    constexpr float kFloor = 0.01f;
    std::vector<float> mag(M);
    for (unsigned int k = 0; k < M; ++k) {
        unsigned int i = validIdx[k];
        m_ftdata[k].frequency = m_source->frequency(i);
        m_ftdata[k].coherence = m_source->coherence(i);
        mag[k] = std::max(m_source->magnitudeRaw(i), kFloor);
    }

    // Update minFrequency and auto-initialise highpass on first use.
    if (!qFuzzyCompare(m_minFrequency, fMin)) {
        m_minFrequency = fMin;
        emit minFrequencyChanged();
    }
    if (m_highpass == 0.f) {
        m_highpass = fMin;
        emit highpassChanged();
    }

    // Step 2 — smooth magnitudes in dB/log-frequency space before inverting,
    // so the kernel operates on the measured shape, not the amplified correction.
    int smoothN = (m_smoothing >= 0 && m_smoothing < 6) ? s_smoothingN[m_smoothing] : 0;
    if (smoothN > 0) {
        mag = applySmoothing(mag, smoothN);
    }

    // Step 3 — invert to correction filter; result is zero-phase (real-valued H).
    std::vector<Complex> H(M);
    for (unsigned int k = 0; k < M; ++k) {
        H[k] = Complex{1.f / mag[k], 0.f};
    }

    // Step 4 — apply LP/HP cutoffs.
    applyCutoffs(H, fMin);

    // Store back to m_ftdata for display
    for (unsigned int k = 0; k < M; ++k) {
        float mod = H[k].abs();
        m_ftdata[k].module    = mod;
        m_ftdata[k].magnitude = mod;   // Data::magnitude(i) returns 20*log10(this)
        m_ftdata[k].phase     = (mod > 1e-12f) ? H[k] * (1.f / mod) : Complex{1.f, 0.f};
    }

    m_source->unlock();
    emit readyRead();
}

// -----------------------------------------------------------------------
std::vector<float> FirSource::applySmoothing(const std::vector<float> &mag, int ppo) const
{
    const int M = static_cast<int>(mag.size());
    if (M == 0) return mag;

    std::vector<float> out(M);

    // Gaussian kernel in log2-frequency space with sigma = 1/(2*ppo) octaves.
    // Smoothing is done in the dB domain so a symmetric kernel in log-frequency
    // maps to symmetric dB weighting — no phase cancellation, no frequency bias.
    const float sigma   = 1.f / (2.f * static_cast<float>(ppo));  // octaves
    const float sigma2  = sigma * sigma;
    const float winHalf = 3.f * sigma;  // cut-off at ±3 sigma (~99.7 %)

    for (int k = 0; k < M; ++k) {
        const float f0 = m_ftdata[k].frequency;
        if (f0 <= 0.f) { out[k] = mag[k]; continue; }

        const float log2_f0 = std::log2(f0);
        const float fLo = std::pow(2.f, log2_f0 - winHalf);
        const float fHi = std::pow(2.f, log2_f0 + winHalf);

        float wSum  = 0.f;
        float accum = 0.f;

        for (int j = 0; j < M; ++j) {
            const float fj = m_ftdata[j].frequency;
            if (fj < fLo || fj > fHi) continue;

            // Distance in log2 space, weight in dB domain
            const float d = std::log2(fj) - log2_f0;
            const float w = std::exp(-0.5f * d * d / sigma2);

            // Accumulate in dB so the average is geometrically correct
            accum += 20.f * std::log10(mag[j]) * w;
            wSum  += w;
        }

        // Convert averaged dB back to linear
        out[k] = (wSum > 0.f)
                 ? std::pow(10.f, accum / (wSum * 20.f))
                 : mag[k];
    }
    return out;
}

// -----------------------------------------------------------------------
void FirSource::applyCutoffs(std::vector<Complex> &H, float fMin) const
{
    // Power-law rolloff: gain = (f/fc)^(slope/6) below HP, (fc/f)^(slope/6) above LP.
    // This gives exactly `slope` dB/octave at every point in the rolloff region.
    // slope/6 converts dB/oct to the Nth-order exponent (6 dB/oct = 1st order).
    const float exponent = m_slope / 6.f;
    const int M = static_cast<int>(H.size());

    for (int i = 0; i < M; ++i) {
        const float f = m_ftdata[i].frequency;
        float gain = 1.f;

        // Hard zero below the minimum representable frequency
        if (f < fMin) {
            gain = 0.f;
        } else {
            // Highpass power-law rolloff below m_highpass
            const float hp = (m_highpass > fMin) ? m_highpass : fMin;
            if (f < hp) {
                gain *= std::pow(f / hp, exponent);
            }

            // Lowpass power-law rolloff above m_lowpass
            if (m_lowpass > 0.f && f > m_lowpass) {
                gain *= std::pow(m_lowpass / f, exponent);
            }
        }

        H[i] *= gain;
    }
}

// -----------------------------------------------------------------------
Complex FirSource::interpolateH(float freq) const
{
    const unsigned int M = frequencyDomainSize();
    if (M == 0) return {};

    // Below first bin
    if (freq <= m_ftdata[0].frequency) {
        return m_ftdata[0].phase * m_ftdata[0].module;
    }
    // Above last bin
    if (freq >= m_ftdata[M - 1].frequency) {
        return m_ftdata[M - 1].phase * m_ftdata[M - 1].module;
    }

    // Binary search for surrounding bins
    unsigned int lo = 0, hi = M - 1;
    while (hi - lo > 1) {
        unsigned int mid = (lo + hi) / 2;
        if (m_ftdata[mid].frequency <= freq) lo = mid;
        else hi = mid;
    }

    float fLo = m_ftdata[lo].frequency;
    float fHi = m_ftdata[hi].frequency;
    float t = (fHi > fLo) ? (freq - fLo) / (fHi - fLo) : 0.f;

    Complex HLo = m_ftdata[lo].phase * m_ftdata[lo].module;
    Complex HHi = m_ftdata[hi].phase * m_ftdata[hi].module;
    return HLo * (1.f - t) + HHi * t;
}

// -----------------------------------------------------------------------
bool FirSource::exportFIR(const QUrl &fileName) const noexcept
{
    const unsigned int N = static_cast<unsigned int>(m_tapCount);
    if (frequencyDomainSize() == 0 || N == 0) return false;

    // Determine sample rate from source impulse data
    unsigned int sr = 48000;
    if (m_source && m_source->timeDomainSize() > 2) {
        float dt = std::abs(m_source->impulseTime(1) - m_source->impulseTime(0));
        if (dt > 0.f) {
            sr = static_cast<unsigned int>(std::round(10.f / dt)) * 100u;
        }
    }

    // Build uniform FFT grid via interpolation from current m_ftdata
    FourierTransform ifft;
    ifft.setType(FourierTransform::Fast);
    ifft.setSampleRate(sr);
    ifft.setSize(N);
    ifft.prepare();

    auto freqList = ifft.getFrequencies();  // N/2 bins: 0..sr/2
    for (unsigned int k = 0; k < freqList.size(); ++k) {
        Complex H = interpolateH(freqList[k]);
        // Conjugate-symmetric fill (following filtersource convention)
        ifft.set(k,         H.conjugate(), Complex{});
        ifft.set(N - k - 1, H,             Complex{});
    }

    ifft.transformSingleChannel();  // reverse = true

    // Normalise, circularly shift to centre, and apply Hann window
    const float norm = 1.f / static_cast<float>(N);
    std::vector<float> taps(N);
    for (unsigned int i = 0; i < N; ++i) {
        unsigned int src = (i + N / 2) % N;
        float w = 0.5f * (1.f - std::cos(2.f * static_cast<float>(M_PI) * i / (N - 1)));
        taps[i] = ifft.af(src).real * norm * w;
    }

    QFile file(fileName.toLocalFile());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("FirSource: couldn't open export file.");
        return false;
    }
    QTextStream out(&file);
    out.setRealNumberPrecision(10);
    for (unsigned int i = 0; i < N; ++i) {
        out << taps[i] << "\n";
    }
    file.close();
    return true;
}
