/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#ifndef SINSWEEP_H
#define SINSWEEP_H

#include "outputdevice.h"
class GeneratorThread;
class SinSweep : public OutputDevice
{
    Q_OBJECT

public:
    explicit SinSweep(GeneratorThread *parent);
    float startFrequency() const
    {
        return m_start;
    }

public slots:
    void setStart(int start);
    void setEnd(int end);
    void setFrequency(int f);
    void setDuration(float duration);
    void enabledChanged(bool);

private:
    Sample sample();

    float m_frequency;
    double m_phase;
    float m_start;
    float m_end;
    float m_duration;

signals:
    void frequencyChanged(int);
};

#endif // SINSWEEP_H
