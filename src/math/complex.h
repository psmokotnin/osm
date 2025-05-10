/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#ifndef COMPLEX_H
#define COMPLEX_H

#include <math.h>
#include <QDebug>

struct complex {

    static const complex i;

    float real { 0 };
    float imag { 0 };

    complex() = default;
    complex (float r, float i = 0.0): real(r), imag(i) {}

    float abs() const noexcept;
    float absSquared() const noexcept;
    float arg() const noexcept;

    const complex conjugate() const;
    const complex normalize() const;
    const complex rotate(const float &a) const;
    void polar(const float &phase);
    void polar(const complex &a, const complex &b);

    complex &operator=(const float &r);

    const complex operator+(const float &r) const;
    const complex operator+(const complex &c) const;

    complex &operator+=(const float &r);
    complex &operator+=(const complex &c);

    const complex operator-(const float &r) const;
    const complex operator-(const complex &c) const;

    complex &operator-=(const float &r);
    complex &operator-=(const complex &c);

    const complex operator/(const float &r) const;
    const complex operator/(const complex &c) const;

    complex &operator/=(const float &r);
    complex &operator/=(const complex &c);

    complex operator*(const float &r) const;
    complex operator*(const complex &c) const;

    complex &operator*=(const float &r);
    complex &operator*=(const complex &c);

    bool operator==(const complex &c) const;
    bool operator!=(const complex &c) const;

    bool operator<(const complex &c) const;
};

QDebug operator<<(QDebug dbg, const complex &c);

#endif // COMPLEX_H
