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

Coherence::Coherence(): m_subpointer(0), m_depth(1)
{

}
void Coherence::setSize(const size_t &size) noexcept
{
    Grr.resize(size);
    Gmm.resize(size);
    Grm.resize(size);
    setDepth(m_depth);
}
void Coherence::setDepth(const size_t &depth) noexcept
{
    m_depth = depth;
    Gmm.each([&depth](container::array<float> *a) {
        a->resize(depth);
        a->fill(0.f);
    });
    Grr.each([&depth](container::array<float> *a) {
        a->resize(depth);
        a->fill(0.f);
    });
    Grm.each([&depth](container::array<complex> *a) {
        a->resize(depth);
        a->fill(0);
    });
}
void Coherence::append(unsigned int i, complex refernce, complex measurement) noexcept
{
    if (i == 0) {
        ++m_subpointer;
        if (m_subpointer >= m_depth)
            m_subpointer = 0;
    }

    Grr[i][m_subpointer] = std::pow(refernce.abs(), 2.f);
    Gmm[i][m_subpointer] = std::pow(measurement.abs(), 2.f);
    Grm[i][m_subpointer] = refernce.conjugate() * measurement;
}
float Coherence::value(unsigned int i) const noexcept
{
    float Crr(0), Cmm(0);
    complex Crm(0);

    for (unsigned int j = 0; j < m_depth; ++j) {
        Crm += Grm[i][j];
        Crr += Grr[i][j];
        Cmm += Gmm[i][j];
    }
    return Crm.abs() / std::sqrt(Crr * Cmm);
}
