#include "complex.h"

Complex::Complex(QObject *parent) : QObject(parent)
{
    value = 0;
}
Complex::Complex(qreal r, QObject *parent) : QObject(parent)
{
    value = r;
}

Complex::Complex(qreal r, qreal i, QObject *parent) : QObject(parent)
{
    value.real(r);
    value.imag(i);;
}

QDebug operator<<(QDebug dbg, const Complex &c)
{
    dbg.nospace() << "Complex value: r:"
        << c.real() << " i:" << c.imag();

    return dbg.maybeSpace();
}
