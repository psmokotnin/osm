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

template <typename T>
struct atomic : public std::atomic<T> {

    atomic(T value = 0) : std::atomic<T>(value) {};
    atomic(const atomic &other) : std::atomic<T>()
    {
        this->store(other.load());
    }
    atomic &operator=(const float value)
    {
        this->store(value);
        return *this;
    }
};

#endif // ATOMIC_H
