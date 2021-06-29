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

#ifdef __GNUC__
#define GNU_ALIGN __attribute__((force_align_arg_pointer))
#else
#define GNU_ALIGN
#endif

#include "ssemath.h"

FourierTransform::FourierTransform(unsigned int size):
    m_size(size),
    m_pointer(0),
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
void FourierTransform::setType(FourierTransform::Type type)
{
    m_type = type;
}
void FourierTransform::setWindowFunctionType(WindowFunction::Type type)
{
    m_window.setType(type);
}
std::vector<float> FourierTransform::getFrequencies(unsigned int sampleRate)
{
    std::vector<float> list;

    switch (m_type) {
    case Fast: {
        list.resize(m_size / 2);
        float kf = static_cast<float>(sampleRate) / m_size;
        for (unsigned int i = 0; i < m_size / 2; ++i) {
            list[i] = static_cast<float>(i * kf);
        }
    }
    break;
    case Log: {
        list.resize(m_logBasis.size());
        for (unsigned int i = 0; i < list.size(); ++i) {
            list[i] = sampleRate * m_logBasis[i].frequency;
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
        m_wlen[l] = complex(cosf(ang), sinf(ang));
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
complex FourierTransform::af(unsigned int i) const
{
    return m_fastA[i];
}
complex FourierTransform::bf(unsigned int i) const
{
    return m_fastB[i];
}
long FourierTransform::f2i(double frequency, int sampleRate) const
{
    return static_cast<long>(frequency * m_size / sampleRate);
}
void FourierTransform::add(float sampleA, float sampleB)
{
    m_pointer++;
    if (m_pointer >= m_size)
        m_pointer = 0;

    m_inA[m_pointer] = sampleA;
    m_inB[m_pointer] = sampleB;
}
void FourierTransform::set(unsigned int i, const complex &a, const complex &b)
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
        for (unsigned int i = 0, n = m_pointer + 1; i < m_size; i++, n++) {
            if (n >= m_size) n = 0;
            m_fastA[m_swapMap[i]] = m_inA[n] * m_window.get(i);
            m_fastB[m_swapMap[i]] = m_inB[n] * m_window.get(i);
        }
    }


    complex w;//, ua, va, ub,vb;
    v4sf vw1, vw2, vu, vv, vr, v1, v2, vwl, norm;
#ifdef _MSC_VER
    __declspec(align(16))
#endif
    float stored[4];
    unsigned long ultraLimit = m_size / 2;
    bool breakloop = false;
    norm = _mm_set1_ps(1.f / sqrtf(m_size));
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
            w = 1.0;
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
                vw1 = _mm_set_ps(w.imag, w.real, w.imag, w.real);
                v2  = _mm_set_ps(m_fastB[t2].imag, -1.f * m_fastB[t2].imag, m_fastA[t2].imag,
                                 -1.f * m_fastA[t2].imag);
                vw2 = _mm_set_ps(w.real, w.imag, w.real, w.imag);

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


                vw1 = _mm_set_ps(w.imag, w.real, w.imag, w.real);

                //w *= wlen[l];
                vw1 = _mm_mul_ps(vw1, vwl);
                _mm_store_ps(stored, vw1);
                w.real = stored[0] - stored[1];
                w.imag = stored[2] + stored[3];
            }
        }
    }
}
GNU_ALIGN void FourierTransform::log()
{
    v4sf data, t, m;
    float stored[4];
    for (unsigned int i = 0; i < m_logBasis.size(); ++i) {

        data = _mm_set1_ps(0.f);

        int pointer = m_pointer - m_logBasis[i].N;
        if (pointer < 0) pointer += m_size;

        for (unsigned int j = 0; j < m_logBasis[i].N; ++j, ++pointer) {
            if (pointer >= m_size) pointer -= m_size;
            //_fastA[i] +=  w * inA[j];
            //_fastB[i] +=  w * inB[j];

            //BUG: A <-> B ??
            t    = _mm_set_ps(m_inB[pointer], m_inB[pointer], m_inA[pointer], m_inA[pointer]);
            m    = _mm_mul_ps(t, m_logBasis[i].w[j]);
            data = _mm_add_ps(data, m);
        }
        _mm_store_ps(stored, data);
        m_fastA[i].real = std::move(stored[0]);
        m_fastA[i].imag = std::move(stored[1]);
        m_fastB[i].real = std::move(stored[2]);
        m_fastB[i].imag = std::move(stored[3]);
    }
}
GNU_ALIGN void FourierTransform::prepareLog()
{
    complex w;
    const int ppo = 24, octaves = 11;
    unsigned int startWindow = pow(2, 16), startOffset = 28;
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

        m_logBasis[i].N = N;
        m_logBasis[i].frequency = frequency;
        m_logBasis[i].w.resize(N);

        float gain(0);
        for (unsigned int j = 0; j < N; ++j) {
            gain += m_window.pointGain(j, N) / N;
        }

        for (unsigned int j = 0; j < N; ++j) {
            w.polar(-2.f  * M_PI * j * frequency);
            w *= m_window.pointGain(j, N) / (sqrtf(N) * gain);
            m_logBasis[i].w[j] = _mm_set_ps(w.real, w.imag, w.real, w.imag);
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
