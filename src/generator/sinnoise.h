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
#ifndef SINNOISE_H
#define SINNOISE_H

#include <QtMath>
#include "outputdevice.h"

class SinNoise : public OutputDevice
{
    Q_OBJECT

public:
    SinNoise(QObject *parent);

public slots:
    void setFrequency(int f);

private:
    Sample sample();

    float m_frequency;
    double m_phase;
};

#endif // SINNOISE_H
