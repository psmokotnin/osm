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
#include "item.h"

namespace remote {

Item::Item(QObject *parent) : chart::Source(parent), m_serverId(nullptr), m_sourceId(nullptr)
{
    setObjectName("remoteItem");
    setActive(true);
}

chart::Source *Item::clone() const
{
    return nullptr;
}

QJsonObject Item::toJSON(const SourceList *) const noexcept
{
    QJsonObject object;
    object["active"]    = active();
    object["name"]      = name();

    object["serverId"]  = serverId().toString();
    object["sourceId"]  = sourceId().toString();

    QJsonObject color;
    color["red"]    = m_color.red();
    color["green"]  = m_color.green();
    color["blue"]   = m_color.blue();
    color["alpha"]  = m_color.alpha();
    object["color"] = color;

    return object;
}

void Item::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    auto jsonColor = data["color"].toObject();
    QColor c(
        jsonColor["red"  ].toInt(0),
        jsonColor["green"].toInt(0),
        jsonColor["blue" ].toInt(0),
        jsonColor["alpha"].toInt(1));
    setColor(c);
    setName(data["name"].toString());
    setActive(data["active"].toBool(active()));

    setServerId(QUuid::fromString(data["serverId"].toString()));
    setSourceId(QUuid::fromString(data["sourceId"].toString()));
}

QUuid Item::serverId() const
{
    return m_serverId;
}

void Item::setServerId(const QUuid &serverId)
{
    m_serverId = serverId;
}

QUuid Item::sourceId() const
{
    return m_sourceId;
}

void Item::setSourceId(const QUuid &dataId)
{
    m_sourceId = dataId;
}

} // namespace remote
