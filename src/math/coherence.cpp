/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#include "coherence.h"
#if defined(Q_PROCESSOR_X86_64)
#include "ssemath.h"
#endif
#if defined(Q_PROCESSOR_ARM)
#include "armmath.h"
#endif

Coherence::Coherence(): m_subpointer(0), m_depth(1)
{

}
void Coherence::setSize(const size_t &size) noexcept
{
    Q_ASSERT(size % 4 == 0);
    m_Grr.resize(size);
    m_Gmm.resize(size);
    m_Grm.resize(size);
    m_Crr.resize(size);
    m_Cmm.resize(size);
    m_Crm.resize(size);
    m_Crr.fill(0);
    m_Cmm.fill(0);
    m_Crm.fill(0);
    setDepth(m_depth);
}
void Coherence::setDepth(const size_t &depth) noexcept
{
    m_depth = depth;
    m_Gmm.each([&depth](Container::array<float> *a) {
        a->resize(depth);
        a->fill(0.f);
    });
    m_Grr.each([&depth](Container::array<float> *a) {
        a->resize(depth);
        a->fill(0.f);
    });
    m_Grm.each([&depth](Container::array<Complex> *a) {
        a->resize(depth);
        a->fill(0);
    });
}
void Coherence::append(unsigned int i, const Complex &refernce, const Complex &measurement) noexcept
{
    if (i == 0) {
        ++m_subpointer;
        if (m_subpointer >= m_depth)
            m_subpointer = 0;
    }

    m_Grr[i][m_subpointer] = std::pow(refernce.abs(), 2.f);
    m_Gmm[i][m_subpointer] = std::pow(measurement.abs(), 2.f);
    m_Grm[i][m_subpointer] = refernce.conjugate() * measurement;
}
float Coherence::value(unsigned int i) const noexcept
{
    float Crr(0), Cmm(0);
    Complex Crm(0);

    for (unsigned int j = 0; j < m_depth; ++j) {
        Crm += m_Grm[i][j];
        Crr += m_Grr[i][j];
        Cmm += m_Gmm[i][j];
    }
    return Crm.abs() / std::sqrt(Crr * Cmm);
}

GNU_ALIGN void Coherence::calculate(Abstract::Source::FTData *dst, FourierTransform *src)
{
    ++m_subpointer;
    if (m_subpointer >= m_depth)
        m_subpointer = 0;

    float Crrmm[4], CrmAbs[4];
    v4sf CrrmmVec, CrmAbsVec;

    for (unsigned int i = 0; i < m_Grr.size() ; i += 4) {

        calculateRR(i, src);
        calculateMM(i, src);
        calculateRM(i, src);

        Crrmm[0] = m_Crr[i    ] * m_Cmm[i    ];
        Crrmm[1] = m_Crr[i + 1] * m_Cmm[i + 1];
        Crrmm[2] = m_Crr[i + 2] * m_Cmm[i + 2];
        Crrmm[3] = m_Crr[i + 3] * m_Cmm[i + 3];

        CrmAbs[0] = m_Crm[i    ].absSquared();
        CrmAbs[1] = m_Crm[i + 1].absSquared();
        CrmAbs[2] = m_Crm[i + 2].absSquared();
        CrmAbs[3] = m_Crm[i + 3].absSquared();

        CrrmmVec = _mm_load_ps( Crrmm );
        CrrmmVec = _mm_mul_ps( CrrmmVec, _mm_rsqrt_ps( CrrmmVec ) );

        CrmAbsVec = _mm_load_ps( CrmAbs );
        CrmAbsVec = _mm_mul_ps( CrmAbsVec, _mm_rsqrt_ps( CrmAbsVec ) );

        CrmAbsVec = _mm_div_ps(CrmAbsVec, CrrmmVec);
        _mm_store_ps(CrmAbs, CrmAbsVec);

        dst[i    ].coherence = CrmAbs[0];
        dst[i + 1].coherence = CrmAbs[1];
        dst[i + 2].coherence = CrmAbs[2];
        dst[i + 3].coherence = CrmAbs[3];
    }
}

void Coherence::calculateRR(unsigned int i, FourierTransform *src)
{
    m_Crr[i    ] -= m_Grr[i    ][m_subpointer];
    m_Crr[i + 1] -= m_Grr[i + 1][m_subpointer];
    m_Crr[i + 2] -= m_Grr[i + 2][m_subpointer];
    m_Crr[i + 3] -= m_Grr[i + 3][m_subpointer];

    m_Grr[i    ][m_subpointer] = src->bf(i    ).absSquared();
    m_Grr[i + 1][m_subpointer] = src->bf(i + 1).absSquared();
    m_Grr[i + 2][m_subpointer] = src->bf(i + 2).absSquared();
    m_Grr[i + 3][m_subpointer] = src->bf(i + 3).absSquared();

    m_Crr[i    ] += m_Grr[i    ][m_subpointer];
    m_Crr[i + 1] += m_Grr[i + 1][m_subpointer];
    m_Crr[i + 2] += m_Grr[i + 2][m_subpointer];
    m_Crr[i + 3] += m_Grr[i + 3][m_subpointer];
}

void Coherence::calculateMM(unsigned int i, FourierTransform *src)
{
    m_Cmm[i    ] -= m_Gmm[i    ][m_subpointer];
    m_Cmm[i + 1] -= m_Gmm[i + 1][m_subpointer];
    m_Cmm[i + 2] -= m_Gmm[i + 2][m_subpointer];
    m_Cmm[i + 3] -= m_Gmm[i + 3][m_subpointer];

    m_Gmm[i    ][m_subpointer] = src->af(i    ).absSquared();
    m_Gmm[i + 1][m_subpointer] = src->af(i + 1).absSquared();
    m_Gmm[i + 2][m_subpointer] = src->af(i + 2).absSquared();
    m_Gmm[i + 3][m_subpointer] = src->af(i + 3).absSquared();

    m_Cmm[i    ] += m_Gmm[i    ][m_subpointer];
    m_Cmm[i + 1] += m_Gmm[i + 1][m_subpointer];
    m_Cmm[i + 2] += m_Gmm[i + 2][m_subpointer];
    m_Cmm[i + 3] += m_Gmm[i + 3][m_subpointer];
}

void Coherence::calculateRM(unsigned int i, FourierTransform *src)
{
    m_Crm[i    ] -= m_Grm[i    ][m_subpointer];
    m_Crm[i + 1] -= m_Grm[i + 1][m_subpointer];
    m_Crm[i + 2] -= m_Grm[i + 2][m_subpointer];
    m_Crm[i + 3] -= m_Grm[i + 3][m_subpointer];

    m_Grm[i    ][m_subpointer] = src->bf(i    ).conjugate() * src->af(i    );
    m_Grm[i + 1][m_subpointer] = src->bf(i + 1).conjugate() * src->af(i + 1);
    m_Grm[i + 2][m_subpointer] = src->bf(i + 2).conjugate() * src->af(i + 2);
    m_Grm[i + 3][m_subpointer] = src->bf(i + 3).conjugate() * src->af(i + 3);

    m_Crm[i    ] += m_Grm[i    ][m_subpointer];
    m_Crm[i + 1] += m_Grm[i + 1][m_subpointer];
    m_Crm[i + 2] += m_Grm[i + 2][m_subpointer];
    m_Crm[i + 3] += m_Grm[i + 3][m_subpointer];
}
