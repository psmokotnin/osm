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
#include "levelobject.h"
#include <QDebug>

namespace Chart {

LevelObject::LevelObject() : m_curve(Weighting::A), m_time(Meter::Slow), m_mode(SPL), m_pause(false)
{
}

QVariant LevelObject::getAvailableCurves() const
{
    return Weighting::availableCurves();
}

QVariant LevelObject::getAvailableTimes() const
{
    return Meter::availableTimes();
}

Weighting::Curve LevelObject::curve() const
{
    return m_curve;
}

QString LevelObject::curveName() const
{
    return Weighting::curveName(m_curve);
}

void LevelObject::setCurve(const QString &curve)
{
    auto newCurve = Weighting::curveByName(curve);
    if (newCurve != m_curve) {
        m_curve = newCurve;
        emit curveChanged(curveName());
    }
}

Meter::Time LevelObject::time() const
{
    return m_time;
}

QString LevelObject::timeName() const
{
    return Meter::timeName(m_time);
}

void LevelObject::setTime(const QString &time)
{
    auto newTime = Meter::timeByName(time);
    if (newTime != m_time) {
        m_time = newTime;
        emit timeChanged(timeName());
    }
}

LevelObject::Mode LevelObject::mode() const
{
    return m_mode;
}

const std::unordered_map<LevelObject::Mode, QString>LevelObject::m_modeMap = {
    {LevelObject::dBfs,  "dBfs"},
    {LevelObject::SPL,   "SPL"}
};

QString LevelObject::modeName() const
{
    try {
        return m_modeMap.at(m_mode);
    } catch (std::exception &e) {
        qDebug() << __FILE__ << ":" << __LINE__  << e.what();
    }
    Q_ASSERT(false);
    return "";
}

void LevelObject::setMode(const Mode &mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        emit modeChanged(m_mode);
    }
}

void LevelObject::setMode(const int &mode)
{
    setMode(static_cast<Mode>(mode));
}

bool LevelObject::pause() const
{
    return m_pause;
}

void LevelObject::setPause(bool pause)
{
    if (m_pause != pause) {

        m_pause = pause;
        emit pauseChanged(m_pause);
    }
}

} // namespace chart
