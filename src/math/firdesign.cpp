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
#include "firdesign.h"
#include "fouriertransform.h"

#include <algorithm>
#include <cmath>

namespace FirDesign {

namespace {

constexpr double       kTwoPi         = 2.0 * M_PI;
constexpr float        kMinMagnitude  = 1e-6f;   // -120 dB, only guards the log of zero
constexpr unsigned int kGridSize      = 65536;   // design resolution, independent of the tap count
constexpr float        kGridPerOctave = 256.f;   // resolution of the smoothing grid

// Bins with coherence below kCoherenceLow get no correction, above kCoherenceHigh the full one.
constexpr float kCoherenceLow  = 0.4f;
constexpr float kCoherenceHigh = 0.8f;

float coherenceWeight(float coherence)
{
    return std::clamp((coherence - kCoherenceLow) / (kCoherenceHigh - kCoherenceLow), 0.f, 1.f);
}

unsigned int nextPowerOfTwo(unsigned int value)
{
    unsigned int result = 1;
    while (result < value) {
        result <<= 1;
    }
    return result;
}

// Standard DFT pair on top of FourierTransform.
class Transform
{
public:
    explicit Transform(unsigned int size) : m_size(size)
    {
        m_ft.setType(FourierTransform::Fast);
        m_ft.setSize(size);
        m_ft.prepare();
    }

    // X[k] = sum x[n] exp(-j 2pi k n / N)
    void forward(std::vector<Complex> &data)
    {
        for (unsigned int i = 0; i < m_size; ++i) {
            m_ft.set(i, data[i], Complex{});
        }
        m_ft.transformSingleChannel(true);
        for (unsigned int i = 0; i < m_size; ++i) {
            data[i] = m_ft.af(i);
        }
    }

    // x[n] = 1/N sum X[k] exp(+j 2pi k n / N)
    // The unnormalised reverse kernel of FourierTransform is exp(-j ...), so the spectrum is fed mirrored.
    void inverse(std::vector<Complex> &data)
    {
        m_ft.set(0, data[0], Complex{});
        for (unsigned int k = 1; k < m_size; ++k) {
            m_ft.set(k, data[m_size - k], Complex{});
        }
        m_ft.transformSingleChannel(true);
        const float norm = 1.f / static_cast<float>(m_size);
        for (unsigned int i = 0; i < m_size; ++i) {
            data[i] = m_ft.af(i) * norm;
        }
    }

private:
    unsigned int     m_size;
    FourierTransform m_ft;
};

// Gaussian smoothing in log2-frequency, sigma in octaves, each bin weighted by weight[j].
// Bins are accumulated on a fine log grid first, so the cost depends on the octave span
// and the smoothing width, not on the number of FFT bins.
void smooth(const std::vector<float> &log2f, const std::vector<float> &weight,
            const std::vector<std::vector<float> *> &channels, float sigma)
{
    const size_t M = log2f.size();
    const size_t C = channels.size();
    if (M < 2 || C == 0 || sigma <= 0.f) {
        return;
    }

    const float  origin = log2f.front();
    const size_t cells  = static_cast<size_t>((log2f.back() - origin) * kGridPerOctave) + 2;

    std::vector<float> cellWeight(cells, 0.f);
    std::vector<float> cellSum(cells * C, 0.f);
    for (size_t j = 0; j < M; ++j) {
        auto g = std::min(static_cast<size_t>(std::lround((log2f[j] - origin) * kGridPerOctave)), cells - 1);
        cellWeight[g] += weight[j];
        for (size_t c = 0; c < C; ++c) {
            cellSum[g * C + c] += weight[j] * (*channels[c])[j];
        }
    }

    // kernel is cut at +-3 sigma (~99.7 %)
    const size_t half = static_cast<size_t>(std::ceil(3.f * sigma * kGridPerOctave));
    std::vector<float> kernel(half + 1);
    for (size_t d = 0; d <= half; ++d) {
        const float x = static_cast<float>(d) / (kGridPerOctave * sigma);
        kernel[d] = std::exp(-0.5f * x * x);
    }

    std::vector<float> smoothed(cells * C, 0.f);
    std::vector<bool>  valid(cells, false);
    std::vector<float> accum(C);
    for (size_t g = 0; g < cells; ++g) {
        float wSum = 0.f;
        std::fill(accum.begin(), accum.end(), 0.f);
        const size_t from = g > half ? g - half : 0;
        const size_t to   = std::min(cells - 1, g + half);
        for (size_t s = from; s <= to; ++s) {
            if (cellWeight[s] <= 0.f) {
                continue;
            }
            const float k = kernel[s > g ? s - g : g - s];
            wSum += k * cellWeight[s];
            for (size_t c = 0; c < C; ++c) {
                accum[c] += k * cellSum[s * C + c];
            }
        }
        if (wSum > 0.f) {
            valid[g] = true;
            for (size_t c = 0; c < C; ++c) {
                smoothed[g * C + c] = accum[c] / wSum;
            }
        }
    }

    // back to the bins: interpolate between the neighbouring cell centres
    for (size_t j = 0; j < M; ++j) {
        const float  position = (log2f[j] - origin) * kGridPerOctave;
        const size_t g0 = std::min(static_cast<size_t>(position), cells - 2);
        const size_t g1 = g0 + 1;
        const float  t  = std::clamp(position - static_cast<float>(g0), 0.f, 1.f);
        for (size_t c = 0; c < C; ++c) {
            auto &value = (*channels[c])[j];
            if (valid[g0] && valid[g1]) {
                value = smoothed[g0 * C + c] * (1.f - t) + smoothed[g1 * C + c] * t;
            } else if (valid[g0]) {
                value = smoothed[g0 * C + c];
            } else if (valid[g1]) {
                value = smoothed[g1 * C + c];
            }
        }
    }
}

// Position of x within the ascending xs as index j and fraction t, clamped to the ends.
// j is a cursor: successive calls must use ascending x.
void locate(const std::vector<float> &xs, float x, size_t &j, float &t)
{
    if (x <= xs.front()) {
        j = 0;
        t = 0.f;
        return;
    }
    if (x >= xs.back()) {
        j = xs.size() - 2;
        t = 1.f;
        return;
    }
    while (xs[j + 1] <= x) {
        ++j;
    }
    const float width = xs[j + 1] - xs[j];
    t = width > 0.f ? (x - xs[j]) / width : 0.f;
}

float lerp(const std::vector<float> &ys, size_t j, float t)
{
    return ys[j] + (ys[j + 1] - ys[j]) * t;
}

} // namespace

float minFrequency(unsigned int sampleRate, unsigned int taps)
{
    return taps ? static_cast<float>(sampleRate) / static_cast<float>(taps) : 0.f;
}

Result design(const Input &input, const Parameters &p)
{
    Result result;
    const size_t M = input.frequency.size();
    if (M < 2 || p.taps < 2 || p.sampleRate == 0 ||
            input.magnitude.size() != M || input.phase.size() != M) {
        return result;
    }

    const unsigned int N       = p.taps;
    const float        nyquist = static_cast<float>(p.sampleRate) / 2.f;
    const bool         fullPhase = (p.phaseMode == FullPhase);

    // ── correction range ────────────────────────────────────────────────────
    const float low  = std::max(p.lowLimit, minFrequency(p.sampleRate, N));
    float       high = std::min(input.frequency.back(), nyquist);
    if (p.highLimit > 0.f) {
        high = std::min(high, p.highLimit);
    }

    const bool hasCoherence = input.coherence.size() == M &&
                              std::any_of(input.coherence.begin(), input.coherence.end(),
                                          [](float c) { return c > 0.f; });

    // ── per bin data in log-frequency / dB space ────────────────────────────
    std::vector<float> log2f(M), binWidth(M), weight(M), level(M), coherence(M, 1.f);
    for (size_t j = 0; j < M; ++j) {
        log2f[j] = std::log2(input.frequency[j]);
        level[j] = 20.f * std::log10(std::max(input.magnitude[j], kMinMagnitude));
        if (hasCoherence) {
            coherence[j] = std::clamp(input.coherence[j], 0.f, 1.f);
        }
    }
    // bins of a linear FFT get denser with frequency: weight each by its width in octaves,
    // so averages are not biased towards the upper side
    for (size_t j = 0; j < M; ++j) {
        const float lo = log2f[j > 0 ? j - 1 : j];
        const float hi = log2f[j + 1 < M ? j + 1 : j];
        binWidth[j] = std::max((hi - lo) / 2.f, 1e-9f);
        weight[j]   = binWidth[j] * (hasCoherence ? std::max(coherence[j], 0.05f) : 1.f);
    }

    // measured phase with the bulk delay removed, as separate channels so it can be smoothed
    std::vector<float> re, im;
    if (fullPhase) {
        re.resize(M);
        im.resize(M);
        for (size_t j = 0; j < M; ++j) {
            const double angle = std::fmod(kTwoPi * input.frequency[j] * input.delay, kTwoPi);
            Complex phasor = input.phase[j];
            const float abs = phasor.abs();
            phasor = abs > 0.f ? (phasor / abs).rotate(static_cast<float>(angle)) : Complex{1.f, 0.f};
            re[j] = phasor.real;
            im[j] = phasor.imag;
        }
    }

    if (p.smoothing > 0) {
        std::vector<std::vector<float> *> channels {&level};
        if (hasCoherence) {
            channels.push_back(&coherence);
        }
        if (fullPhase) {
            channels.push_back(&re);
            channels.push_back(&im);
        }
        smooth(log2f, weight, channels, 1.f / (2.f * static_cast<float>(p.smoothing)));
    }

    std::vector<float> confidence(M, 1.f);
    if (hasCoherence) {
        for (size_t j = 0; j < M; ++j) {
            confidence[j] = coherenceWeight(coherence[j]);
        }
    }

    // ── reference level: mean level inside the correction range ─────────────
    double wSum = 0, lSum = 0;
    for (size_t j = 0; j < M; ++j) {
        if (input.frequency[j] < low || input.frequency[j] > high) {
            continue;
        }
        const double w = binWidth[j] * confidence[j];
        wSum += w;
        lSum += w * level[j];
    }
    if (wSum <= 0) {
        for (size_t j = 0; j < M; ++j) {
            wSum += binWidth[j];
            lSum += binWidth[j] * level[j];
        }
    }
    const float reference = static_cast<float>(lSum / wSum);

    // ── excess phase, unwrapped ─────────────────────────────────────────────
    std::vector<float> excess(M, 0.f);
    if (fullPhase) {
        Complex previous{re[0], im[0]};
        excess[0] = previous.arg();
        for (size_t j = 1; j < M; ++j) {
            const Complex current{re[j], im[j]};
            excess[j] = excess[j - 1] + (current * previous.conjugate()).arg();
            previous  = current;
        }

        // drop the arbitrary multiple of 2pi the unwrapping started with
        double pSum = 0, pWeight = 0;
        for (size_t j = 0; j < M; ++j) {
            if (input.frequency[j] >= low && input.frequency[j] <= high) {
                pSum    += binWidth[j] * confidence[j] * excess[j];
                pWeight += binWidth[j] * confidence[j];
            }
        }
        if (pWeight > 0) {
            const float offset = static_cast<float>(kTwoPi * std::round(pSum / pWeight / kTwoPi));
            for (auto &value : excess) {
                value -= offset;
            }
        }
    }

    // ── correction at the measured bins ─────────────────────────────────────
    std::vector<float> gain(M), phase(M, 0.f);
    for (size_t j = 0; j < M; ++j) {
        gain[j] = std::clamp(reference - level[j], -p.maxCut, p.maxBoost);
        if (fullPhase) {
            phase[j] = -excess[j];
        }
    }

    // ── target spectrum on a fine uniform grid ──────────────────────────────
    // The correction is faded towards unity (0 dB, 0 rad) outside the range, and
    // dB and phase are scaled separately so the fade can never cancel itself.
    const unsigned int G  = std::max(kGridSize, nextPowerOfTwo(8 * N));
    const float        df = static_cast<float>(p.sampleRate) / static_cast<float>(G);
    Transform transform(G);

    std::vector<Complex> spectrum(G);
    size_t cursor = 0;
    for (unsigned int k = 0; k <= G / 2; ++k) {
        const float f = k * df;
        float fade = 0.f;
        if (k > 0) {
            fade = 1.f;
            if (f < low) {
                fade *= std::pow(f / low, p.transition);
            }
            if (f > high) {
                fade *= std::pow(high / f, p.transition);
            }
        }

        float t;
        locate(input.frequency, f, cursor, t);
        const float amount = fade * lerp(confidence, cursor, t);
        const float dB     = lerp(gain, cursor, t) * amount;
        const float angle  = (k == G / 2) ? 0.f : lerp(phase, cursor, t) * amount;
        const float abs    = std::pow(10.f, dB / 20.f);
        spectrum[k] = Complex{abs * std::cos(angle), abs * std::sin(angle)};
    }
    for (unsigned int k = G / 2 + 1; k < G; ++k) {
        spectrum[k] = spectrum[G - k].conjugate();
    }

    if (p.phaseMode == MinimumPhase) {
        // homomorphic method: fold the real cepstrum of log|H| onto positive quefrencies
        std::vector<Complex> cepstrum(G);
        for (unsigned int k = 0; k < G; ++k) {
            cepstrum[k] = Complex{std::log(spectrum[k].abs())};
        }
        transform.inverse(cepstrum);
        for (unsigned int n = 0; n < G; ++n) {
            float folded = 0.f;
            if (n == 0 || n == G / 2) {
                folded = cepstrum[n].real;
            } else if (n < G / 2) {
                folded = 2.f * cepstrum[n].real;
            }
            cepstrum[n] = Complex{folded};
        }
        transform.forward(cepstrum);
        for (unsigned int k = 0; k < G; ++k) {
            const float abs = std::exp(cepstrum[k].real);
            spectrum[k] = Complex{abs * std::cos(cepstrum[k].imag), abs * std::sin(cepstrum[k].imag)};
        }
    }

    // ── impulse at full resolution, then truncate and window to N taps ──────
    transform.inverse(spectrum);

    result.taps.resize(N);
    if (p.phaseMode == MinimumPhase) {
        // causal: keep the start, fade out the tail with a half Hann window
        result.latency = 0;
        for (unsigned int i = 0; i < N; ++i) {
            const float w = 0.5f * (1.f + std::cos(static_cast<float>(M_PI) * i / N));
            result.taps[i] = spectrum[i].real * w;
        }
    } else {
        // centred on tap N/2 with a periodic Hann window, which is symmetric around N/2
        result.latency = N / 2;
        for (unsigned int i = 0; i < N; ++i) {
            const float w = 0.5f * (1.f - std::cos(2.f * static_cast<float>(M_PI) * i / N));
            result.taps[i] = spectrum[(i + G - result.latency) % G].real * w;
        }
    }

    // ── realised response of the taps, bulk delay removed ───────────────────
    std::vector<Complex> realised(G, Complex{});
    for (unsigned int i = 0; i < N; ++i) {
        realised[(i + G - result.latency) % G] = result.taps[i];
    }
    transform.forward(realised);

    result.response.resize(M);
    for (size_t j = 0; j < M; ++j) {
        const float  position = std::clamp(input.frequency[j] / df, 0.f, static_cast<float>(G / 2));
        const auto   k0 = std::min(static_cast<unsigned int>(position), G / 2 - 1);
        const float  t  = position - static_cast<float>(k0);
        result.response[j] = realised[k0] * (1.f - t) + realised[k0 + 1] * t;
    }
    return result;
}

} // namespace FirDesign
