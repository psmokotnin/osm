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
#ifndef BROWNNOISE_H
#define BROWNNOISE_H

#include "outputdevice.h"
#include <QRandomGenerator>

class BrownNoise : public OutputDevice
{
    Q_OBJECT

public:
    explicit BrownNoise(QObject *parent = nullptr);

private:
    Sample sample() override;
    QRandomGenerator m_generator;
    Sample m_lastSample;
};

#endif // BROWNNOISE_H
