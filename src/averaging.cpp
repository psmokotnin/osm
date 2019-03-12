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
#include <cmath>
#include "averaging.h"

template <> void Averaging<complex>::checkDepth(unsigned int i) {
    while (data.pat(i)->size() > m_depth) {
        m_value[i] -= data.pat(i)->front();
        data.pat(i)->pop();
        m_collected[i]--;
    }
};
template<> void Averaging<complex>::append(unsigned int i, const complex &value)
{
    data.pat(i)->push(value);
    m_value[i] += value;
    m_collected[i]++;
    checkDepth(i);
};
template <> complex Averaging<complex>::value(unsigned int i)
{
    if (m_collected[i] == 0)
        return complex(0);

    return m_value[i] / (m_collected[i] * m_gain);
};

template <> void Averaging<float>::checkDepth(unsigned int i) {
    while (data.pat(i)->size() > m_depth) {
        m_value[i] -= data.pat(i)->front();
        data.pat(i)->pop();
        m_collected[i]--;
    }
};
template<> void Averaging<float>::append(unsigned int i, const float &value)
{
    data.pat(i)->push(value);
    m_value[i] += value;
    m_collected[i]++;
    checkDepth(i);
};
template <> float Averaging<float>::value(unsigned int i)
{
    if (m_collected[i] == 0)
        return 0.f;

    return m_value[i] / (m_collected[i] * m_gain);
};

template <> void Averaging<unsigned int>::checkDepth(unsigned int i) {
    while (data.pat(i)->size() > m_depth) {
        m_value[i] -= data.pat(i)->front();
        data.pat(i)->pop();
        m_collected[i]--;
    }
};
template<> void Averaging<unsigned int>::append(unsigned int i, const unsigned int &value)
{
    data.pat(i)->push(value);
    m_value[i] += value;
    m_collected[i]++;
    checkDepth(i);
};
template <> unsigned int Averaging<unsigned int>::value(unsigned int i)
{
    if (m_collected[i] == 0)
        return 0;

    return m_value[i] / m_collected[i];
};
