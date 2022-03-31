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
#include <QJsonArray>

namespace remote {

Item::Item(QObject *parent) : chart::Source(parent), m_serverId(nullptr), m_sourceId(nullptr),
    m_state(WAIT), m_stateTimer()
{
    setObjectName("remoteItem");
    setActive(true);

    m_stateTimer.setSingleShot(true);
    m_stateTimer.setInterval(1000);
    connect(&m_stateTimer, &QTimer::timeout, this, &Item::resetState);
    connect(this, &Item::stateChanged, this, &Item::startResetTimer);
    connect(this, &Source::activeChanged, this, &Item::refresh);
}

chart::Source *Item::clone() const
{
    return nullptr;
}

bool Item::cloneable() const
{
    return false;
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

bool Item::originalActive() const
{
    return m_originalActive;
}

void Item::setOriginalActive(bool originalActive)
{
    m_originalActive = originalActive;
}

void Item::applyData(const QJsonArray &data)
{
    std::lock_guard guard(m_dataMutex);
    //m_deconvolutionSize = static_cast<unsigned int>(impulse.count());
    //m_impulseData        = new TimeData[m_deconvolutionSize];
    if (m_dataLength != static_cast<unsigned int>(data.count())) {
        m_dataLength         = static_cast<unsigned int>(data.count());
        m_ftdata             = new FTData[m_dataLength];
    }

    for (int i = 0; i < data.count(); i++) {
        auto row = data[i].toArray();
        if (row.count() > 0) m_ftdata[i].frequency    = static_cast<float>(row[0].toDouble());
        if (row.count() > 1) m_ftdata[i].module       = static_cast<float>(row[1].toDouble());
        if (row.count() > 2) m_ftdata[i].magnitude    = static_cast<float>(row[2].toDouble());
        if (row.count() > 3) m_ftdata[i].phase.polar(   static_cast<float>(row[3].toDouble()));
        if (row.count() > 4) m_ftdata[i].coherence    = static_cast<float>(row[4].toDouble());
        if (row.count() > 5) m_ftdata[i].peakSquared  = static_cast<float>(row[5].toDouble());
        if (row.count() > 6) m_ftdata[i].meanSquared  = static_cast<float>(row[6].toDouble());
    }
    emit readyRead();
    setState(UPDATED);
}

Item::State Item::state() const
{
    return m_state;
}

void Item::setState(const State &state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged();
    }
}

void Item::startResetTimer()
{
    if (m_state == UPDATED) {
        m_stateTimer.start();
    }
}

void Item::resetState()
{
    if (m_state == UPDATED) {
        setState(WAIT);
    }
}

QString Item::host() const
{
    return m_host;
}

void Item::setHost(const QString &host)
{
    if (m_host != host) {
        m_host = host;
        emit hostChanged();
    }
}

void Item::refresh()
{
    emit updateData(this);
}

} // namespace remote
