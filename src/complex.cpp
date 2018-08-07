#include "complex.h"

float complex::abs()
{
    return sqrtf(powf(real, 2) + powf(imag, 2));
}
float complex::arg()
{
    return atan2f(imag, real);
}
const complex complex::conjugate() const
{
    complex rc;
    rc.real = real;
    rc.imag = -1 * imag;
    return rc;
}
void complex::polar(const float& phase)
{
    real = cos(phase);
    imag = sin(phase);
}
complex& complex::operator=(const float& r)
{
    real = r;
    imag = 0.0;
    return *this;
}
complex& complex::operator=(const complex& c)
{
    real = c.real;
    imag = c.imag;
    return *this;
}

const complex complex::operator+(const float& r) const
{
    complex rc;
    rc.real = real + r;
    rc.imag = imag;
    return rc;
}
const complex complex::operator+(const complex& c) const
{
    complex rc;
    rc.real = real + c.real;
    rc.imag = imag + c.imag;
    return rc;
}

complex& complex::operator+=(const float& r)
{
    real += r;
    return *this;
}
complex& complex::operator+=(const complex& c)
{
    real += c.real;
    imag += c.imag;
    return *this;
}

const complex complex::operator-(const float& r) const
{
    complex rc;
    rc.real = real - r;
    rc.imag = imag;
    return rc;
}
const complex complex::operator-(const complex& c) const
{
    complex rc;
    rc.real = real - c.real;
    rc.imag = imag - c.imag;
    return rc;
}

complex& complex::operator-=(const float& r)
{
    real -= r;
    return *this;
}
complex& complex::operator-=(const complex& c)
{
    real -= c.real;
    imag -= c.imag;
    return *this;
}

const complex complex::operator/(const float& r) const
{
    complex rc;
    rc.real = real / r;
    rc.imag = imag / r;
    return rc;
}
const complex complex::operator/(const complex& c) const
{
    complex rc;
    float d = c.real * c.real + c.imag * c.imag;

    rc.real = (real * c.real + imag * c.imag) / d;
    rc.imag = (imag * c.real - real * c.imag) / d;

    return rc;
}
complex& complex::operator/=(const float& r)
{
    real /= r;
    imag /= r;
    return *this;
}
complex& complex::operator/=(const complex& c)
{
    float r = real, d = c.real * c.real + c.imag * c.imag;

    real = (real * c.real + imag * c.imag) / d;
    imag = (imag * c.real - r * c.imag) / d;

    return *this;
}

complex complex::operator*(const float& r) const
{
    complex rc;
    rc.real = real * r;
    rc.imag = imag * r;
    return rc;
}
complex complex::operator*(const complex& c) const
{
    complex rc;
    rc.real = real * c.real - imag * c.imag;
    rc.imag = real * c.imag + imag * c.real;
    return rc;
}

complex& complex::operator*=(const float& r)
{
    real *= r;
    imag *= r;
    return *this;
}
complex& complex::operator*=(const complex& c)
{
    float r = real;
    real = real * c.real - imag * c.imag;
    imag = r * c.imag + imag * c.real;
    return *this;
}
bool complex::operator==(const complex& c) const
{
    return (real == c.real && imag == c.imag);
}
bool complex::operator!=(const complex& c) const
{
    return (real != c.real || imag != c.imag);
}
QDebug operator<<(QDebug dbg, const complex &c)
{
    dbg.nospace() << "Complex value: r:"
        << c.real << " i:" << c.imag;

    return dbg.maybeSpace();
}
