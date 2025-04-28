/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#ifndef CONTAINER_CIRCULAR_H
#define CONTAINER_CIRCULAR_H

#include <vector>

namespace Container {
template<typename T> class Circular
{
public:
    Circular(size_t size) : m_size(size), m_read(0), m_write(0), m_collected(0), m_data()
    {
        m_data.resize(m_size, 0);
    }

    void operator=(Circular &other)
    {
        m_size = other.m_size.load();
        clear();
    }

    const T &read()
    {
        if (m_collected == 0) {
            static const T t {0};
            return t;
        }

        T *v = m_data.data() + m_read;
        ++m_read;
        --m_collected;
        if (m_read >= m_size) {
            m_read = 0;
        }
        return *v;
    }

    void write(const T &value)
    {
        T *v = m_data.data() + m_write;
        *v = value;
        ++m_write;
        ++m_collected;
        if (m_write >= m_size) {
            m_write = 0;
        }
    }

    const T replace(const T &value)
    {
        T *v_ptr = m_data.data() + m_write;
        T v = *v_ptr;
        *v_ptr = value;
        ++m_write;
        if (m_write >= m_size) {
            m_write = 0;
        }
        return v;
    }

    void reset()
    {
        m_read = 0;
        m_write = 0;
        m_collected = 0;
    }

    void clear()
    {
        m_data.clear();
        reset();
    }

    size_t collected() const
    {
        return m_collected;
    }

    size_t size() const
    {
        return m_size;
    }

    void resize(size_t size)
    {
        m_size = size;
        m_data.resize(m_size, 0);
        clear();
    }
private:
    std::atomic<size_t> m_size;
    std::atomic<size_t> m_read, m_write, m_collected;
    std::vector<T> m_data;
};
}

#endif // CONTAINER_CIRCULAR_H
