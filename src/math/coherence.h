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
#ifndef COHERENCE_H
#define COHERENCE_H

#include <cmath>
#include "./complex.h"
#include "container/array.h"
#include "abstract/source.h"
#include "fouriertransform.h"

class Coherence
{
private:
    Container::array<Container::array<float>> m_Grr, m_Gmm;
    Container::array<Container::array<Complex>> m_Grm;
    unsigned int m_subpointer;
    size_t m_depth;

    Container::array<float> m_Crr, m_Cmm;
    Container::array<Complex> m_Crm;

public:
    Coherence();

    void setDepth(const size_t &depth) noexcept;
    void setSize(const size_t &size) noexcept;
    [[deprecated]] void append(unsigned int i, const Complex &refernce, const Complex &measurement) noexcept;
    [[deprecated]] float value(unsigned int i) const noexcept;

    void calculate(Abstract::Data::FTData *dst, FourierTransform *src);
    inline void calculateRR(unsigned int i, FourierTransform *src);
    inline void calculateMM(unsigned int i, FourierTransform *src);
    inline void calculateRM(unsigned int i, FourierTransform *src);
};

#endif // COHERENCE_H
