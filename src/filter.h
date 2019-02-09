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
#ifndef FILTER_H
#define FILTER_H

#include "complex.h"

class Filter
{
    unsigned int _p = 3;
    complex _x[6], _y[6];
    unsigned int p(unsigned int i) const noexcept;
    const complex &x(unsigned int i) const noexcept;
    const complex &y(unsigned int i) const noexcept;

public:
    explicit Filter() {}
    complex operator()(const complex &v);

};

#endif // FILTER_H
