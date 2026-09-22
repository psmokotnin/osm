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
      m_lowpass(0.f),
      m_tapCount(512)
{
    setObjectName("FirSource");
    setName("FIR Export");

    connect(this, &FirSource::sourceChanged,   this, &FirSource::update);
    connect(this, &FirSource::smoothingChanged, this, &FirSource::update);
    connect(this, &FirSource::highpassChanged,  this, &FirSource::update);
    connect(this, &FirSource::lowpassChanged,   this, &FirSource::update);
    connect(this, &FirSource::tapCountChanged,  this, &FirSource::update);
}

Shared::Source FirSource::clone() const
{
    auto cloned = std::make_shared<FirSource>(parent());
    cloned->setSource(source());
    cloned->setSmoothing(m_smoothing);
    cloned->setHighpass(m_highpass);
    cloned->setLowpass(m_lowpass);
    cloned->setTapCount(m_tapCount);
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
    setLowpass( static_cast<float>(data["lowpass" ].toDouble(0.0)));
    setTapCount(data["tapCount"].toInt(512));

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
    const unsigned int M = m_source->frequencyDomainSize();
    setFrequencyDomainSize(M);

    // Build complex H from source FTData
    std::vector<Complex> H(M);
    for (unsigned int i = 0; i < M; ++i) {
        m_ftdata[i].frequency = m_source->frequency(i);
        m_ftdata[i].coherence = m_source->coherence(i);
        // H = module * unit-phasor
        H[i] = m_source->phase(i) * m_source->module(i);
    }

    // Apply smoothing in complex domain
    int smoothN = (m_smoothing >= 0 && m_smoothing < 6) ? s_smoothingN[m_smoothing] : 0;
    if (smoothN > 0) {
        H = applySmoothing(H, smoothN);
    }

    // Apply LP/HP cutoffs
    applyCutoffs(H);

    // Store back to m_ftdata for display (magnitude chart etc.)
    for (unsigned int i = 0; i < M; ++i) {
        float mod = H[i].abs();
        m_ftdata[i].module    = mod;
        m_ftdata[i].magnitude = mod;   // Data::magnitude(i) returns 20*log10(this)
        m_ftdata[i].phase     = (mod > 1e-12f) ? H[i] * (1.f / mod) : Complex{1.f, 0.f};
    }

    m_source->unlock();
    emit readyRead();
}

// -----------------------------------------------------------------------
std::vector<Complex> FirSource::applySmoothing(const std::vector<Complex> &H, int smoothingN) const
{
    const int M = static_cast<int>(H.size());
    std::vector<Complex> out(M);

    const double bwFactor = 1.0 / smoothingN;           // bandwidth in octaves
    const double sigmaLog2 = bwFactor / 4.0;            // Gaussian sigma in log2 space

    for (int k = 0; k < M; ++k) {
        const double f0 = static_cast<double>(m_ftdata[k].frequency);
        if (f0 <= 0.0) { out[k] = H[k]; continue; }

        const double fLo = f0 * std::pow(2.0, -bwFactor / 2.0);
        const double fHi = f0 * std::pow(2.0,  bwFactor / 2.0);

        Complex accum{};
        double wSum = 0.0;

        for (int j = 0; j < M; ++j) {
            const double fj = static_cast<double>(m_ftdata[j].frequency);
            if (fj < fLo || fj > fHi) continue;

            double logDist = std::log2(fj / f0);
            double w = std::exp(-0.5 * (logDist / sigmaLog2) * (logDist / sigmaLog2));

            accum += H[j] * static_cast<float>(w);
            wSum  += w;
        }

        out[k] = (wSum > 1e-12) ? accum * static_cast<float>(1.0 / wSum) : H[k];
    }
    return out;
}

// -----------------------------------------------------------------------
void FirSource::applyCutoffs(std::vector<Complex> &H) const
{
    const float taperOctaves = 0.5f;
    const int M = static_cast<int>(H.size());

    for (int i = 0; i < M; ++i) {
        const float f = m_ftdata[i].frequency;
        float gain = 1.f;

        // Highpass: roll off below m_highpass
        if (m_highpass > 0.f && f < m_highpass) {
            const float fEdge = m_highpass * std::pow(2.f, -taperOctaves);
            if (f <= fEdge) {
                gain = 0.f;
            } else {
                float t = std::log2(f / fEdge) / taperOctaves;
                gain *= 0.5f * (1.f - std::cos(static_cast<float>(M_PI) * t));
            }
        }

        // Lowpass: roll off above m_lowpass
        if (m_lowpass > 0.f && f > m_lowpass) {
            const float fEdge = m_lowpass * std::pow(2.f, taperOctaves);
            if (f >= fEdge) {
                gain = 0.f;
            } else {
                float t = std::log2(fEdge / f) / taperOctaves;
                gain *= 0.5f * (1.f - std::cos(static_cast<float>(M_PI) * t));
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
