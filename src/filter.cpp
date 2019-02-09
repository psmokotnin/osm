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
#include "filter.h"

unsigned int Filter::p(unsigned int i) const noexcept
{
    unsigned int p = _p + i;
    if (p > 5) p -= 6;
    return p;
}
const complex &Filter::x(unsigned int i) const noexcept
{
    return _x[p(i)];
}
const complex &Filter::y(unsigned int i) const noexcept
{
    return _y[p(i)];
}
/**
 * @url http://www-users.cs.york.ac.uk/~fisher/cgi-bin/mkfscript
 */
complex Filter::operator()(const complex &v)
{
    _p = p(1);

    _x[p(5)] = v / static_cast<float>(1.327313202e+05);
    _y[p(5)] =
           ( x(0) * 1.0)
         + ( x(1) * 5.0)
         + ( x(2) * 10.0)
         + ( x(3) * 10.0)
         + ( x(4) * 5.0)
         + ( x(5) * 1.0)

         + ( y(0) * static_cast<float>( 0.4600089841 ))
         + ( y(1) * static_cast<float>(-2.6653917847 ))
         + ( y(2) * static_cast<float>( 6.2006547950 ))
         + ( y(3) * static_cast<float>(-7.2408808951 ))
         + ( y(4) * static_cast<float>( 4.2453678122 ));

    return y(5);
}
