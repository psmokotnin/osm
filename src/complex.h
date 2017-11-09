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

    complex& operator=(const float& r);
    complex& operator=(const complex& c);

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
