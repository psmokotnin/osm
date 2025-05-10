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

struct Complex {

    static const Complex i;

    float real { 0 };
    float imag { 0 };

    Complex() = default;
    Complex (float r, float i = 0.0): real(r), imag(i) {}

    float abs() const noexcept;
    float absSquared() const noexcept;
    float arg() const noexcept;

    const Complex conjugate() const;
    const Complex normalize() const;
    const Complex rotate(const float &a) const;
    void polar(const float &phase);
    void polar(const Complex &a, const Complex &b);

    Complex &operator=(const float &r);

    const Complex operator+(const float &r) const;
    const Complex operator+(const Complex &c) const;

    Complex &operator+=(const float &r);
    Complex &operator+=(const Complex &c);

    const Complex operator-(const float &r) const;
    const Complex operator-(const Complex &c) const;

    Complex &operator-=(const float &r);
    Complex &operator-=(const Complex &c);

    const Complex operator/(const float &r) const;
    const Complex operator/(const Complex &c) const;

    Complex &operator/=(const float &r);
    Complex &operator/=(const Complex &c);

    Complex operator*(const float &r) const;
    Complex operator*(const Complex &c) const;

    Complex &operator*=(const float &r);
    Complex &operator*=(const Complex &c);

    bool operator==(const Complex &c) const;
    bool operator!=(const Complex &c) const;

    bool operator<(const Complex &c) const;
};

QDebug operator<<(QDebug dbg, const Complex &c);

#endif // COMPLEX_H
