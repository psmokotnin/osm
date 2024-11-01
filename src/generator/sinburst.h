/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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
#ifndef SINBURST_H
#define SINBURST_H

#include "outputdevice.h"

class SinBurst : public OutputDevice
{
    Q_OBJECT
public:
    explicit SinBurst(QObject *parent = nullptr);

public slots:
    void setFrequency(int f);

private:
    Sample sample() override;

    float m_frequency;
    double m_phase;

    int m_periods;
    bool m_burst;
};

#endif // SINBURST_H
