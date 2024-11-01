/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#ifndef CHART_LEVEL_H
#define CHART_LEVEL_H

#include <unordered_map>
#include "math/weighting.h"
#include "math/meter.h"

namespace Chart {

class LevelObject
{
    Q_GADGET
public:
    enum Mode {
        dBfs    = 0x00,
        SPL     = 0x01
    };
    Q_ENUM(Chart::LevelObject::Mode);

    static constexpr float SPL_OFFSET = 140;

    LevelObject();

    QVariant getAvailableCurves() const;
    virtual QVariant getAvailableTimes() const;

    Weighting::Curve curve() const;
    QString curveName() const;
    void setCurve(const QString &curve);

    Meter::Time time() const;
    virtual QString timeName() const;
    virtual void setTime(const QString &time);

    Chart::LevelObject::Mode mode() const;
    QString modeName() const;
    void setMode(const Chart::LevelObject::Mode &mode);
    void setMode(const int &mode);

    bool pause() const;
    void setPause(bool pause);

//virtual signals:
    virtual void curveChanged(QString) = 0;
    virtual void timeChanged(QString) = 0;
    virtual void modeChanged(Chart::LevelObject::Mode) = 0;
    virtual void pauseChanged(bool) = 0;

private:
    Weighting::Curve m_curve;
    Meter::Time m_time;
    Mode m_mode;
    bool m_pause;

    static const std::unordered_map<Mode, QString> m_modeMap;
};

} // namespace chart

Q_DECLARE_METATYPE(Chart::LevelObject::Mode);

#endif // CHART_LEVEL_H
