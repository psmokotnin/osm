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
#include "stored.h"

Stored::Stored(QObject *parent) : Fftchart::Source(parent)
{

}
void Stored::build (Fftchart::Source *source)
{
    _dataLength = source->size();
    _deconvolutionSize = source->impulseSize();
    setFftSize(source->fftSize());
    _ftdata = new FTData[_dataLength];
    _impulseData = new TimeData[_deconvolutionSize];
    source->copy(_ftdata, _impulseData);
    emit readyRead();
}
