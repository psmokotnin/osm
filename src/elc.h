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
#ifndef ELC_H
#define ELC_H

#include <QObject>
#include "chart/source.h"

class ELC : public chart::Source
{
    Q_OBJECT
    Q_PROPERTY(float loudness READ loudness WRITE setLoudness NOTIFY loudnessChanged)

public:
    explicit ELC(QObject *parent = nullptr);
    Source *clone() const override;

    Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    float loudness() const noexcept
    {
        return m_loudness;
    }
    void setLoudness(float loudness);

private:
    float m_loudness;

    void update();

signals:
    void loudnessChanged(float);
};

#endif // ELC_H
