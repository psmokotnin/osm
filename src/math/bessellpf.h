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
#ifndef MATH_FILTER_H
#define MATH_FILTER_H

#include "complex.h"

namespace Filter {

Q_NAMESPACE
/*
 * Bessel Low Pass Filter 5th order
 */
enum Frequency {FourthHz, HalfHz, OneHz};
Q_ENUM_NS(Frequency)

template <typename T> class BesselLPF
{
public:
    static const unsigned int ORDER = 5;

private:
    unsigned int _p = 3;
    T _x[ORDER + 1], _y[ORDER + 1];

    float _gain, _k[ORDER];

    unsigned int p(unsigned int i) const noexcept
    {
        unsigned int p = _p + i;
        if (p > 5) p -= 6;
        return p;
    }
    const T &x(unsigned int i) const noexcept
    {
        return _x[p(i)];
    }
    const T &y(unsigned int i) const noexcept
    {
        return _y[p(i)];
    }

public:
    explicit BesselLPF()
    {
        setFrequency(Frequency::FourthHz);
        reset();
    }

    void setFrequency(Frequency frequency) noexcept
    {
        switch (frequency) {
        case Frequency::FourthHz :
            _gain =  1.327313202e+05f;
            _k[0] =  0.4600089841f;
            _k[1] = -2.6653917847f;
            _k[2] =  6.2006547950f;
            _k[3] = -7.2408808951f;
            _k[4] =  4.2453678122f;
            break;
        case Frequency::HalfHz :
            _gain =  5.908173436e+03f;
            _k[0] =  0.2116396822f;
            _k[1] = -1.3993115731f;
            _k[2] =  3.7525227570f;
            _k[3] = -5.1097576527f;
            _k[4] =  3.5394905611f;
            break;
        case Frequency::OneHz :
            _gain =  3.508023803e+02f;
            _k[0] =  0.0448577871f;
            _k[1] = -0.3690099172f;
            _k[2] =  1.2719460080f;
            _k[3] = -2.3219218420f;
            _k[4] =  2.2829085146;
            break;
        }
        reset();
    }
    void reset() noexcept
    {
        _x[0] = _x[1] = _x[2] = _x[3] = _x[4] = _x[5] = T(0);
        _y[0] = _y[1] = _y[2] = _y[3] = _y[4] = _y[5] = T(0);
    }
    /**
     * @url http://www-users.cs.york.ac.uk/~fisher/cgi-bin/mkfscript
     */
    T operator()(const T &v)
    {
        if (v != v) {   //isnan
            return y(5);
        }
        _p = p(1);

        _x[p(5)] = v / _gain;
        _y[p(5)] =
            (   x(0) * 1.f)
            + ( x(1) * 5.f)
            + ( x(2) * 10.f)
            + ( x(3) * 10.f)
            + ( x(4) * 5.f)
            + ( x(5) * 1.f)

            + ( y(0) * _k[0])
            + ( y(1) * _k[1])
            + ( y(2) * _k[2])
            + ( y(3) * _k[3])
            + ( y(4) * _k[4]);

        return y(5);
    }

};
}
#endif // MATH_FILTER_H
