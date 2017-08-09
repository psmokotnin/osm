#ifndef COMPLEX_H
#define COMPLEX_H

#include <complex>

#include <QObject>
#include <QDebug>

class Complex : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal real READ real);
    Q_PROPERTY(qreal imag READ imag);

private:
    std::complex<qreal> value;

public:
    explicit Complex(QObject *parent = nullptr);
    explicit Complex(qreal r, QObject *parent = nullptr);
    explicit Complex(qreal r, qreal i, QObject *parent = nullptr);

    qreal real() const {return value.real();}
    qreal imag() const {return value.imag();}

signals:

public slots:
};

QDebug operator<<(QDebug dbg, const Complex &c);

#endif // COMPLEX_H
