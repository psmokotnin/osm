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
#ifndef AVERAGING_H
#define AVERAGING_H

#include <queue>
#include "complex.h"
#include "container/fifo.h"
#include "container/array.h"

template<typename T> class Averaging
{

private:
    typedef Container::Fifo<T> dataT;
    Container::array<dataT> m_data;

    Container::array<T> m_value;
    Container::array<unsigned int> m_collected;
    float m_gain;
    unsigned int m_size;
    unsigned int m_depth;

public:
    Averaging():
        m_data(),
        m_value(),
        m_collected(),
        m_gain(1.f),
        m_size(1),
        m_depth(1) {}

    void append(unsigned int i, const T &value);
    T value(unsigned int i);

    void setSize(unsigned int size)
    {
        m_size = size;
        m_data.resize(m_size);
        m_value.resize(m_size);
        m_collected.resize(m_size);

        reset();
    }
    unsigned int size() const
    {
        return m_size;
    }

    void setDepth(unsigned int depth)
    {
        m_depth = depth;
    }
    unsigned int depth() const
    {
        return m_depth;
    }

    float gain() const
    {
        return m_gain;
    }

    void reset()
    {
        m_value.fill(T(0));
        m_data.each([](auto * p) {
            p->reset();
        });
        m_collected.fill(0);
    }

protected:
    void checkDepth(unsigned int i);
};

template<> void Averaging<unsigned int>::append(unsigned int i, const unsigned int &value);
template<> void Averaging<float>::append(unsigned int i, const float &value);
template<> void Averaging<Complex>::append(unsigned int i, const Complex &value);

template <> unsigned int Averaging<unsigned int>::value(unsigned int i);
template <> float Averaging<float>::value(unsigned int i);
template <> Complex Averaging<Complex>::value(unsigned int i);


#endif // AVERAGING_H
