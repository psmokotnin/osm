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
#include "fouriertransform.h"
#include <QtMath>
#ifndef USE_SSE2
#define USE_SSE2
#endif

FourierTransform::FourierTransform(unsigned int size):
    m_size(size),
    m_pointer(0),
    m_sampleRate(1),
    m_logWindowDenominator(1),
    m_type(Fast),
    m_window(WindowFunction::Rectangular),
    m_inA(m_size, 0.f),
    m_inB(m_size, 0.f)
{

}
void FourierTransform::setSize(unsigned int size)
{
    if (m_size != size) {
        m_size = size;
        m_inA.resize(m_size, 0.f);
        m_inB.resize(m_size, 0.f);
    }
}

unsigned int FourierTransform::size() const
{
    return m_size;
}
void FourierTransform::setType(FourierTransform::Type type)
{
    m_type = type;
}
FourierTransform::Type FourierTransform::type() const
{
    return m_type;
}
void FourierTransform::setWindowFunctionType(WindowFunction::Type type)
{
    m_window.setType(type);
}
std::vector<float> FourierTransform::getFrequencies()
{
    std::vector<float> list;

    switch (m_type) {
    case Fast: {
        list.resize(m_size / 2);
        float kf = static_cast<float>(sampleRate()) / m_size;
        for (unsigned int i = 0; i < m_size / 2; ++i) {
            list[i] = static_cast<float>(i * kf);
        }
    }
    break;
    case Log: {
        list.resize(m_logBasis.size());
        for (unsigned int i = 0; i < list.size(); ++i) {
            list[i] = sampleRate() * m_logBasis[i].frequency;
        }
    }
    break;
    }
    return list;
}
void FourierTransform::prepareFast()
{
    m_fastA.resize(m_size);
    m_fastB.resize(m_size);
    m_swapMap.resize(m_size);
    m_window.setSize(m_size);

    auto power = static_cast<unsigned int>(log2(m_size));
    m_wlen.resize(power);
    float ang, dPi = 2 * static_cast<float>(M_PI);
    for (unsigned int len = 2, l = 0; len <= m_size; len <<= 1, l++) {
        ang = dPi / len;
        m_wlen[l] = Complex(cosf(ang), sinf(ang));
    }

    for (unsigned int i = 0; i < m_size; i++) {
        m_swapMap[i] = i;
    }

    for (unsigned int i = 1, j = 0; i < m_size; ++i) {
        unsigned int bit = m_size >> 1;
        for (; j >= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j) {
            std::swap (m_swapMap[i], m_swapMap[j]);
        }
    }
}
Complex FourierTransform::af(unsigned int i) const
{
    return m_fastA[i];
}
Complex FourierTransform::bf(unsigned int i) const
{
    return m_fastB[i];
}

unsigned int FourierTransform::sampleRate() const
{
    return m_sampleRate;
}

void FourierTransform::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
}

void FourierTransform::reset()
{
    for (unsigned int i = 0; i < m_size; ++i) {
        m_inA[i] = m_inB[i] = 0;
    }
}

void FourierTransform::setNorm(Norm newNorm)
{
    m_norm = newNorm;
}

void FourierTransform::setAlign(Align newAlign)
{
    m_align = newAlign;
}

void FourierTransform::setLogWindowDenominator(unsigned int newLogWindowDenominator)
{
    m_logWindowDenominator = newLogWindowDenominator;
}

long FourierTransform::f2i(double frequency, int sampleRate) const
{
    return static_cast<long>(frequency * m_size / sampleRate);
}

unsigned long FourierTransform::pointer() const
{
    return m_pointer;
}

float FourierTransform::aIn() const
{
    return m_inA[m_pointer];
}

float FourierTransform::bIn() const
{
    return m_inB[m_pointer];
}
void FourierTransform::add(float sampleA, float sampleB)
{
    m_pointer++;
    if (m_pointer >= m_size)
        m_pointer = 0;

    m_inA[m_pointer] = sampleA;
    m_inB[m_pointer] = sampleB;
}
void FourierTransform::set(unsigned int i, const Complex &a, const Complex &b)
{
    m_fastA[m_swapMap[i]] = a;
    m_fastB[m_swapMap[i]] = b;
}
void FourierTransform::transform(bool ultra)
{
    switch (m_type) {
    case Fast:
        fast(false, ultra);
        break;
    case Log:
        log();
        break;
    }
}
void FourierTransform::reverse()
{
    Q_ASSERT(m_type == Fast);
    fast(true);
}

GNU_ALIGN void FourierTransform::fast(bool reverse, bool ultrafast)
{
    if (!reverse) {
        //apply data-window
        float m_integratedA = 0;
        float m_integratedB = 0;
        for (unsigned int i = 0, n = m_pointer + 1; i < m_size; i++, n++) {
            if (n >= m_size) n = 0;
            m_fastA[m_swapMap[i]] = m_inA[n] * m_window.get(i);
            m_fastB[m_swapMap[i]] = m_inB[n] * m_window.get(i);

            m_integratedA += m_fastA[m_swapMap[i]].real;
            m_integratedB += m_fastB[m_swapMap[i]].real;
        }

        for (unsigned int i = 0; i < m_size; i++) {
            m_fastA[i] -= m_integratedA;
            m_fastB[i] -= m_integratedB;
        }
    }

    v4sf vw1, vw2, vu, vv, vr, v1, v2, vwl, norm;

    v4sf tmp0, tmp1, tmp2, tmp3, vw;
    v4sf tmpi = _mm_set_ps(1, -1, 1, -1);

#if defined(_MSC_VER)
    __declspec(align(16))
#elif defined(Q_PROCESSOR_ARM)
    __attribute__((aligned(16)))
#endif
    float stored[4];
    unsigned long ultraLimit = m_size / 2;
    bool breakloop = false;

    float normalization = 1.f;
    if (!reverse) {
        normalization /= m_size;
    }
    norm = _mm_set1_ps(normalization);
    for (unsigned int len = 2, l = 0; len <= m_size; len <<= 1, l++, breakloop = false) {
        vwl = _mm_set_ps(
                  m_wlen[l].real,                               //vwl[0] = vwl[3] = wlen[l].real;
                  reverse ? -1 * m_wlen[l].imag :
                  m_wlen[l].imag, //vwl[1] = vwl[2] = reverse ? -1 * wlen[l].imag : wlen[l].imag;
                  reverse ? -1 * m_wlen[l].imag : m_wlen[l].imag,
                  m_wlen[l].real
              );

        for (unsigned int i = 0, t1 = 0, t2 = len / 2; i < m_size
                && !breakloop; i += len, t1 = i, t2 = i + len / 2) {
            // w = 1.0;
            vw  = _mm_set_ps(0, -1, 0, 1);

            for (unsigned long j = 0; j < len / 2; ++j, ++t1, ++t2) {
                //t1 = i + j
                //t2 = i + j + len / 2

                //TODO: investigate depth skips
                if (ultrafast && len == m_size && t1 > ultraLimit) {
                    breakloop = true;
                    break;
                }
                //va = _fastA[i + j + len / 2] * w;
                //vb = _fastB[i + j + len / 2] * w;
                v1  = _mm_set_ps(m_fastB[t2].real, m_fastB[t2].real, m_fastA[t2].real, m_fastA[t2].real);

                //vw1 = _mm_set_ps(w.imag, w.real, w.imag, w.real);
                vw1 = _mm_shuffle_ps(vw, vw, _MM_SHUFFLE(1, 0, 1, 0));

                v2  = _mm_set_ps(m_fastB[t2].imag, -1.f * m_fastB[t2].imag, m_fastA[t2].imag,
                                 -1.f * m_fastA[t2].imag);

                //vw2 = _mm_set_ps(w.real, w.imag, w.real, w.imag);
                vw2 = _mm_shuffle_ps(vw, vw, _MM_SHUFFLE(0, 1, 0, 1));

                v1 = _mm_mul_ps(v1, vw1);
                v2 = _mm_mul_ps(v2, vw2);
                vv = _mm_add_ps(v1, v2);

                //ua = _fastA[i + j];
                //ub = _fastB[i + j];
                vu = _mm_set_ps(m_fastB[t1].imag, m_fastB[t1].real, m_fastA[t1].imag, m_fastA[t1].real);

                //_fastA[i + j] = ua + va;
                //_fastB[i + j] = ub + vb;
                vr = _mm_add_ps(vu, vv);
                if (len == m_size) { //final data normalized
                    vr = _mm_mul_ps(vr, norm);
                }
                _mm_store_ps(stored, vr);
                m_fastA[t1].real = std::move(stored[0]);
                m_fastA[t1].imag = std::move(stored[1]);
                m_fastB[t1].real = std::move(stored[2]);
                m_fastB[t1].imag = std::move(stored[3]);


                //_fastA[i + j + len / 2] = ua - va;
                //_fastB[i + j + len / 2] = ub - vb;
                vr = _mm_sub_ps(vu, vv);
                if (len == m_size) { //final data normalized
                    vr = _mm_mul_ps(vr, norm);
                }
                _mm_store_ps(stored, vr);
                m_fastA[t2].real = std::move(stored[0]);
                m_fastA[t2].imag = std::move(stored[1]);
                m_fastB[t2].real = std::move(stored[2]);
                m_fastB[t2].imag = std::move(stored[3]);

                //w *= wlen[l];
                tmp0 = _mm_shuffle_ps(vw,   vw, _MM_SHUFFLE(2, 2, 0, 0));
                tmp1 = _mm_shuffle_ps(vwl, vwl, _MM_SHUFFLE(1, 0, 1, 0));

                tmp2 = _mm_shuffle_ps(vw,   vw, _MM_SHUFFLE(3, 3, 1, 1));
                tmp3 = _mm_shuffle_ps(vwl, vwl, _MM_SHUFFLE(0, 1, 0, 1));

                tmpi = _mm_set_ps(1, -1, 1, -1);
                tmp2 = _mm_mul_ps(tmp2, tmpi);

                vw = _mm_add_ps(
                         _mm_mul_ps(tmp0, tmp1),
                         _mm_mul_ps(tmp2, tmp3)
                     );
            }
        }
    }
}

GNU_ALIGN void FourierTransform::transformSingleChannel(bool reverse)
{
    v4sf vw1, vw2, vu, vv, vr, v1, v2, vwl, norm;
    v4sf tmp0, tmp1, tmp2, tmp3, vw;
    v4sf tmpi = _mm_set_ps(1, -1, 1, -1);

#ifdef _MSC_VER
    __declspec(align(16))
#endif
    float stored[4];

    float normalization = 1.f;
    if (!reverse) {
        normalization /= m_size;
    }
    norm = _mm_set1_ps(normalization);
    for (unsigned int len = 2, l = 0; len <= m_size; len <<= 1, l++) {
        vwl = _mm_set_ps(
                  m_wlen[l].real,                               //vwl[0] = vwl[3] = wlen[l].real;
                  reverse ? -1 * m_wlen[l].imag :
                  m_wlen[l].imag, //vwl[1] = vwl[2] = reverse ? -1 * wlen[l].imag : wlen[l].imag;
                  reverse ? -1 * m_wlen[l].imag : m_wlen[l].imag,
                  m_wlen[l].real
              );

        for (unsigned int i = 0, t1 = 0, t2 = len / 2; i < m_size; i += len, t1 = i, t2 = i + len / 2) {
            // w = 1.0;
            vw  = _mm_set_ps(0, -1, 0, 1);

            for (unsigned long j = 0; j < len / 2; ++j, ++t1, ++t2) {
                //t1 = i + j
                //t2 = i + j + len / 2

                //va = _fastA[i + j + len / 2] * w;
                //vb = _fastB[i + j + len / 2] * w;
                v1  = _mm_set_ps(m_fastA[t2].real, m_fastA[t2].real,
                                 m_fastA[t2].real, m_fastA[t2].real);

                //vw1 = _mm_set_ps(-w.imag, -w.real, w.imag, w.real);
                vw1 = _mm_shuffle_ps(vw, vw, _MM_SHUFFLE(3, 2, 1, 0));

                v2  = _mm_set_ps(m_fastA[t2].imag, -1.f * m_fastA[t2].imag,
                                 m_fastA[t2].imag, -1.f * m_fastA[t2].imag);

                //vw2 = _mm_set_ps(-w.real, -w.imag, w.real, w.imag);
                vw2 = _mm_shuffle_ps(vw, vw, _MM_SHUFFLE(2, 3, 0, 1));

                v1 = _mm_mul_ps(v1, vw1);
                v2 = _mm_mul_ps(v2, vw2);
                vv = _mm_add_ps(v1, v2);

                //ua = _fastA[i + j];
                //ub = _fastB[i + j];
                vu = _mm_set_ps(m_fastA[t1].imag, m_fastA[t1].real, m_fastA[t1].imag, m_fastA[t1].real);

                //_fastA[i + j] = ua + va;
                //_fastB[i + j] = ub + vb;
                vr = _mm_add_ps(vu, vv);
                if (len == m_size) { //final data normalized
                    vr = _mm_mul_ps(vr, norm);
                }
                _mm_store_ps(stored, vr);
                //ua + va;
                m_fastA[t1].real = std::move(stored[0]);
                m_fastA[t1].imag = std::move(stored[1]);

                //ua - va;
                m_fastA[t2].real = std::move(stored[2]);
                m_fastA[t2].imag = std::move(stored[3]);

                //w *= wlen[l];
                tmp0 = _mm_shuffle_ps(vw,   vw, _MM_SHUFFLE(2, 2, 0, 0));
                tmp1 = _mm_shuffle_ps(vwl, vwl, _MM_SHUFFLE(1, 0, 1, 0));

                tmp2 = _mm_shuffle_ps(vw,   vw, _MM_SHUFFLE(3, 3, 1, 1));
                tmp3 = _mm_shuffle_ps(vwl, vwl, _MM_SHUFFLE(0, 1, 0, 1));

                tmpi = _mm_set_ps(1, -1, 1, -1);
                tmp2 = _mm_mul_ps(tmp2, tmpi);

                vw = _mm_add_ps(
                         _mm_mul_ps(tmp0, tmp1),
                         _mm_mul_ps(tmp2, tmp3)
                     );
            }
        }
    }
}

void FourierTransform::ufast()
{
    fast(false, true);
}

GNU_ALIGN void FourierTransform::log()
{
    v4sf data, t, m;
    float stored[4];
    for (unsigned int i = 0; i < m_logBasis.size(); ++i) {

        data = _mm_set1_ps(0.f);

        int pointer = static_cast<int>(m_pointer);
        switch (m_align) {
        case Center:
            pointer -= m_size / 2;
            pointer -= m_logBasis[i].N / 2;
            break;
        case Right:
            pointer -= m_logBasis[i].N;
            break;
        }

        while (pointer < 0) {
            pointer += m_size;
        }

        for (unsigned int j = 0; j < m_logBasis[i].N; ++j, ++pointer) {
            if (pointer >= static_cast<int>(m_size)) pointer -= m_size;
            //_fastA[i] +=  w * inA[j];
            //_fastB[i] +=  w * inB[j];
            t    = _mm_set_ps(m_inA[pointer], m_inA[pointer], m_inB[pointer], m_inB[pointer]);
            m    = _mm_mul_ps(t, m_logBasis[i].w[j]);
            data = _mm_add_ps(data, m);
        }
        _mm_store_ps(stored, data);
        m_fastA[i].real = std::move(stored[3]);
        m_fastA[i].imag = std::move(stored[2]);
        m_fastB[i].real = std::move(stored[1]);
        m_fastB[i].imag = std::move(stored[0]);
    }
}
GNU_ALIGN void FourierTransform::prepareLog()
{
    Complex w;
    const int ppo = 24, octaves = 11;
    unsigned int startWindow = pow(2, 16), startOffset = 1'344'000 / sampleRate(); // 28 for 48k
    float wFactor = powf(10.f, 1.f / (-octaves * ppo / 2.5));
    float fFactor = powf(1000.f, 1.f / (ppo * octaves));
    unsigned int N, offset;
    float frequency;
    m_logBasis.resize(ppo * octaves);
    m_fastA.resize(ppo * octaves);
    m_fastB.resize(ppo * octaves);
    setSize(startWindow);

    for (unsigned int i = 0; i < m_logBasis.size(); ++i) {
        N      = startWindow * pow(wFactor, i);
        offset = startOffset * pow(wFactor * fFactor, i);
        frequency =  static_cast<float>(offset) / (N);

        m_logBasis[i].N = N / m_logWindowDenominator;
        m_logBasis[i].frequency = frequency;
        m_logBasis[i].w.resize(N);
        float gain(0);
        for (unsigned int j = 0; j < m_logBasis[i].N; ++j) {
            gain += m_window.pointGain(j, m_logBasis[i].N) / m_logBasis[i].N;
        }
        auto norm = (m_norm == Norm::Sqrt ? m_logBasis[i].N : float(1.f) );
        float phase = (m_align == Align::Center ? -(m_logBasis[i].N / 2.f) : 0);
        for (unsigned int j = 0; j < m_logBasis[i].N; ++j, ++phase) {
            w.polar(-2.f  * M_PI * phase * frequency);
            w *= m_window.pointGain(j, m_logBasis[i].N) / (norm * gain);
            m_logBasis[i].w[j] = _mm_set_ps(w.imag, w.real, w.imag, w.real);
        }
    }
}
void FourierTransform::prepare()
{
    switch (m_type) {
    case Fast:
        prepareFast();
        break;
    case Log:
        prepareLog();
        break;
    }
}
