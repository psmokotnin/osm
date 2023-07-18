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

#include <numeric>
#include "biquad.h"
namespace math {
BiQuad::BiQuad() : m_a(), m_b(), m_x(), m_y()
{
}

float BiQuad::operator()(const float &value)
{
    for (int i = 2; i > 0; --i) {
        m_x[i] = m_x[i - 1];
        m_y[i] = m_y[i - 1];
    }
    m_x[0] = value;
    m_y[0] = 0;

    auto y_iter = m_y.begin() + 1;
    float y = std::accumulate(m_a.cbegin() + 1, m_a.cend(), 0.f, [&](float acc, float value) {
        acc += value * (*y_iter);
        y_iter++;
        return acc;
    });

    auto x_iter = m_x.begin();
    float x = std::accumulate(m_b.cbegin(), m_b.cend(), 0.f, [&](float acc, float value) {
        acc += value * (*x_iter);
        x_iter++;
        return acc;
    });

    m_y[0] = (x - y) / m_a[0];
    return m_y[0];
}
}
