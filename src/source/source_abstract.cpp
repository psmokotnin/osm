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
#include <algorithm>
#include "source/source_abstract.h"

namespace Source {

Abstract::Abstract(QObject *parent) : ::Abstract::Source(parent),
    m_onReset(false)
{
    qRegisterMetaType<::Source::Abstract *>("Source*");
}

Abstract::~Abstract()
{
}

void Abstract::destroy()
{
    emit beforeDestroy(this);
    disconnect();
    //deleteLater();   //Schedules ~Source() from qml
}

Source::Shared Abstract::store()
{
    return {};
}

QJsonObject Abstract::toJSON(const SourceList *) const noexcept
{
    QJsonObject object;
    object["uuid"]      = uuid().toString();
    object["active"]    = active();
    object["name"]      = name();

    auto m_color = color();
    QJsonObject jscolor;
    jscolor["red"]    = m_color.red();
    jscolor["green"]  = m_color.green();
    jscolor["blue"]   = m_color.blue();
    jscolor["alpha"]  = m_color.alpha();
    object["color"] = jscolor;

    return object;
}

void Abstract::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    auto uuid = QUuid::fromString(data["uuid"].toString());
    if (!uuid.isNull()) {
        setUuid(uuid);
    }
    setActive(data["active"].toBool(active()));
    setName(data["name"].toString());

    auto jsonColor = data["color"].toObject();
    QColor c(
        jsonColor["red"  ].toInt(0),
        jsonColor["green"].toInt(0),
        jsonColor["blue" ].toInt(0),
        jsonColor["alpha"].toInt(1));
    setColor(c);
}

QJsonObject Abstract::levels()
{
    QJsonObject levels;
    for (auto &&[key, value] : m_levelsData) {

        auto curve = Weighting::curveName(key.curve);
        auto time = Meter::timeName(key.time);

        auto curveData = levels[curve].toObject();
        curveData[time] = level(key.curve, key.time);
        levels[curve] = curveData;
    }
    return levels;
}

void Abstract::setLevels(const QJsonObject &data)
{
    for (auto &&[key, value] : m_levelsData) {

        auto curve = Weighting::curveName(key.curve);
        auto time = Meter::timeName(key.time);

        auto curveData = data[curve].toObject();
        value = curveData[time].toDouble();
    }
}
}
