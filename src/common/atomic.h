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

#ifndef ATOMIC_H
#define ATOMIC_H

#include <atomic>

template <typename T,
          std::enable_if_t<std::atomic<T>::is_always_lock_free, bool> = true
          >
struct Atomic : public std::atomic<T> {

    Atomic(T value = 0) noexcept : std::atomic<T>(value) {}
    ~Atomic() = default;

    Atomic(const Atomic &other) noexcept : std::atomic<T>()
    {
        this->store(other.load());
    }

    Atomic &operator=(const Atomic<T> &value) noexcept
    {
        this->store(value.load());
        return *this;
    }


    Atomic &operator=(const T value) noexcept
    {
        this->store(value);
        return *this;
    }
};

#endif // ATOMIC_H
