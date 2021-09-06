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

class Coherence
{
private:
    container::array<container::array<float>> m_Grr, m_Gmm;
    container::array<container::array<complex>> m_Grm;
    unsigned int m_subpointer;
    size_t m_depth;

public:
    Coherence();

    void setDepth(const size_t &depth) noexcept;
    void setSize(const size_t &size) noexcept;
    void append(unsigned int i, complex refernce, complex measurement) noexcept;
    float value(unsigned int i) const noexcept;
};

#endif // COHERENCE_H
