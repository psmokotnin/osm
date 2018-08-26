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
#include "meter.h"

Meter::Meter(unsigned long size) : AudioStack(size)
{

}
void Meter::add(const float data)
{
    float d = abs(data);
    AudioStack::add(d);
    _squareValue += d;
}
void Meter::dropFirst()
{
    if (firstdata) {
        _squareValue -= firstdata->value;
    }
    AudioStack::dropFirst();
}
float Meter::value() const noexcept
{
    return 20*log10(_squareValue / _size);//dBV
}
