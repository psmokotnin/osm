/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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

#ifndef BIQUAD_H
#define BIQUAD_H

#include <array>
#include "math/filter.h"

namespace math {

struct BiQuad : public math::Filter {

    BiQuad();
    float operator()(const float &value) override;

    std::array<float, 3> m_a, m_b, m_x, m_y;
};
}

#endif // BIQUAD_H
