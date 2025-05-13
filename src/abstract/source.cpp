/**
 *  OSM
 *  Copyright (C) 2025  Pavel Smokotnin

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

#include "source.h"

namespace Abstract {

Source::Source(QObject *parent) : QObject{ parent },
    m_name          {},
    m_color         {},
    m_uuid          { QUuid::createUuid() },
    m_sampleRate    { 48000 },
    m_active        { false },
    m_cloneable     { true  }
{
    qRegisterMetaType<::Abstract::Source *>("AbstractSource*");
}

Source::~Source() = default;

Shared::Source Source::store()
{
    return {};
}

void Source::destroy()
{
    emit beforeDestroy(this);
    disconnect();
}

QJsonObject Source::toJSON() const noexcept
{
    QJsonObject object;
    object["uuid"]      = uuid().toString();
    object["active"]    = active();
    object["name"]      = name();
    object["sampleRate"] = static_cast<int>(sampleRate());

    auto m_color = color();
    QJsonObject jscolor;
    jscolor["red"]    = m_color.red();
    jscolor["green"]  = m_color.green();
    jscolor["blue"]   = m_color.blue();
    jscolor["alpha"]  = m_color.alpha();
    object["color"] = jscolor;

    return object;
}

void Source::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    auto uuid = QUuid::fromString(data["uuid"].toString());
    if (!uuid.isNull()) {
        setUuid(uuid);
    }
    setActive(    data["active"].toBool(active()));
    setName(      data["name"].toString());
    setSampleRate(data["sampleRate"].toInt(       sampleRate()));

    auto jsonColor = data["color"].toObject();
    QColor c(
        jsonColor["red"  ].toInt(0),
        jsonColor["green"].toInt(0),
        jsonColor["blue" ].toInt(0),
        jsonColor["alpha"].toInt(1));
    setColor(c);
}

QJsonObject Source::levels()
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

void Source::setLevels(const QJsonObject &data)
{
    for (auto &&[key, value] : m_levelsData) {

        auto curve = Weighting::curveName(key.curve);
        auto time = Meter::timeName(key.time);

        auto curveData = data[curve].toObject();
        value = curveData[time].toDouble();
    }
}

QString Source::name() const
{
    return m_name;
}

void Source::setName(const QString &newName)
{
    if (m_name == newName)
        return;
    m_name = newName;
    emit nameChanged(m_name);
}

QColor Source::color() const
{
    return m_color;
}

void Source::setColor(const QColor &newColor)
{
    if (m_color == newColor)
        return;
    m_color = newColor;
    emit colorChanged(m_color);
}

bool Source::isColorValid()
{
    return m_color.isValid();
}

QUuid Source::uuid() const
{
    return m_uuid;
}

void Source::setUuid(const QUuid &newUuid)
{
    m_uuid = newUuid;
}

unsigned int Source::sampleRate() const
{
    return m_sampleRate;
}

void Source::setSampleRate(unsigned int newSampleRate)
{
    if (m_sampleRate == newSampleRate) {
        return;
    }

    m_sampleRate = newSampleRate;
    emit sampleRateChanged(m_sampleRate);
}

bool Source::active() const
{
    return m_active;
}

void Source::setActive(bool newActive)
{
    if (m_active == newActive)
        return;
    m_active = newActive;
    emit activeChanged();
}

bool Source::cloneable() const
{
    return m_cloneable;
}

void Source::setGlobalColor(int globalValue)
{
    if (globalValue < 19) {
        setColor(Qt::GlobalColor(globalValue));
    }
}

} // namespace Abstract
