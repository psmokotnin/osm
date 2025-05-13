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
#ifndef STANDARDLINE_H
#define STANDARDLINE_H

#include <QObject>
#include "abstract/source.h"

class StandardLine : public Abstract::Source
{
public:
    enum Mode {
        ELC         = 0,
        WEIGHTING_A = 1,
        WEIGHTING_B = 2,
        WEIGHTING_C = 3
    };

    Q_OBJECT
    Q_ENUM(Mode);
    Q_PROPERTY(float loudness READ loudness WRITE setLoudness NOTIFY loudnessChanged)
    Q_PROPERTY(StandardLine::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)

public:
    explicit StandardLine(QObject *parent = nullptr);
    Shared::Source clone() const override;

    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

    float loudness() const noexcept;
    void setLoudness(float loudness);

    Mode mode() const;
    void setMode(const int &mode);
    void setMode(const Mode &mode);
    QVariant getAvailableModes() const;

signals:
    void loudnessChanged(float);
    void modeChanged(StandardLine::Mode);

private:
    void update();
    void createELC();
    void createWeighting();

    Mode m_mode;
    float m_loudness;

    static const std::map<Mode, QString> m_modeMap;
};

#endif // STANDARDLINE_H
