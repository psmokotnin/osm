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

struct complex
{
    float real;
    float imag;
    complex (float r = 0.0, float i = 0.0): real(r), imag(i) {}
    complex (const complex &c) : real(c.real), imag(c.imag) {}

    float abs();
    float arg();

    const complex conjugate() const;
    void polar(const float &phase);

    complex& operator=(const float& r);

    const complex operator+(const float& r) const;
    const complex operator+(const complex& c) const;

    complex& operator+=(const float& r);
    complex& operator+=(const complex& c);

    const complex operator-(const float& r) const;
    const complex operator-(const complex& c) const;

    complex& operator-=(const float& r);
    complex& operator-=(const complex& c);

    const complex operator/(const float& r) const;
    const complex operator/(const complex& c) const;

    complex& operator/=(const float& r);
    complex& operator/=(const complex& c);

    complex operator*(const float& r) const;
    complex operator*(const complex& c) const;

    complex& operator*=(const float& r);
    complex& operator*=(const complex& c);

    bool operator==(const complex& c) const;
    bool operator!=(const complex& c) const;
};

QDebug operator<<(QDebug dbg, const complex &c);

#endif // COMPLEX_H
