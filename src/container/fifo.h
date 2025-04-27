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
#ifndef FIFO_H
#define FIFO_H

#include <cmath>
#include <atomic>

namespace Container {
template<typename T> class Fifo
{

private:
    struct Cell {
        T value;
        Cell *next = nullptr;
    };

    Cell *m_front, *m_end;
    std::atomic<size_t> m_size;

public:
    Fifo(): m_front(nullptr), m_end(nullptr), m_size(0)
    {
    }

    ~Fifo()
    {
        reset();
    }

    void reset()
    {
        while (m_front) {
            pop();
        }
    }

    void push(const T &value)
    {
        Cell *p = new Cell;
        p->value = value;
        if (m_end) {
            m_end->next = p;
        }
        m_end = p;
        if (!m_front) {
            m_front = p;
        }
        ++m_size;
    }

    T pop()
    {
        if (!m_front) {
            return static_cast<T>(NAN);
        }

        T value = m_front->value;
        Cell *next = m_front->next;

        if (m_end == m_front) {
            m_end = nullptr;
        }
        delete m_front;
        m_front = next;
        --m_size;

        return value;
    }

    /*
     * pop if size > limit, else just push
     */
    T pushnpop(const T value, const size_t &limit)
    {
        if (m_size >= limit && m_front && m_end) {
            T popValue = std::move(m_front->value);
            Cell *reserved = m_front;
            m_front = m_front->next;
            reserved->next = nullptr;
            reserved->value = value;
            m_end->next = reserved;
            m_end = reserved;

            return std::move(popValue);
        }
        push(value);
        return T(0);
    }

    void pushLimit(const T value, const size_t &limit)
    {
        if (m_size >= limit && m_front && m_end) {
            Cell *reserved = m_front;
            m_front = m_front->next;
            reserved->next = nullptr;
            reserved->value = value;
            m_end->next = reserved;
            m_end = reserved;
            return;
        }
        push(value);
    }

    T front() const
    {
        if (!m_front) {
            return T(0);
        }

        return m_front->value;
    }

    size_t size() const
    {
        return m_size;
    }
};

}
#endif // FIFO_H
