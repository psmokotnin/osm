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
#include <QMetaProperty>

namespace remote {

Item::Item(QObject *parent) : Abstract::Source(parent), m_originalActive(false), m_eventSilence(true),
    m_serverId(nullptr), m_sourceId(nullptr), m_state(WAIT), m_stateTimer()
{
    setObjectName("RemoteItem");
    setActive(true);

    m_stateTimer.setSingleShot(true);
    m_stateTimer.setInterval(1000);
    connect(&m_stateTimer, &QTimer::timeout, this, &Item::resetState);
    connect(this, &Item::stateChanged, this, &Item::startResetTimer);
    connect(this, &Abstract::Source::activeChanged, this, &Item::refresh);
}

void Item::connectProperties()
{
    for (int i = 0 ; i < metaObject()->propertyCount(); ++i) {
        auto property = metaObject()->property(i);
        auto signal = property.notifySignal();
        auto normalizedSignature = QMetaObject::normalizedSignature("properiesChanged()");
        auto slotId = metaObject()->indexOfMethod(normalizedSignature);
        if (signal.isValid()) {
            connect(this, signal, this, metaObject()->method(slotId), Qt::DirectConnection);
        }
    }
}

Shared::Source Item::clone() const
{
    return { nullptr };
}

bool Item::cloneable() const
{
    return false;
}

QJsonObject Item::toJSON() const noexcept
{
    QJsonObject object;
    object["serverId"]  = serverId().toString();
    object["sourceId"]  = sourceId().toString();

    return object;
}

void Item::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    setServerId(QUuid::fromString(data["serverId"].toString()));
    setSourceId(QUuid::fromString(data["sourceId"].toString()));
}

QJsonObject Item::metaJsonObject(QString propertyName) const
{
    QJsonObject object {};
    for (int i = 0 ; i < metaObject()->propertyCount(); ++i) {
        auto property = metaObject()->property(i);
        if (!propertyName.isEmpty() && property.name() != propertyName) {
            continue;
        }

        switch (static_cast<int>(property.type())) {

        case QVariant::Type::Bool:
            object[property.name()]  = property.read(this).toBool();
            break;

        case QVariant::Type::UInt:
        case QVariant::Type::Int:
        case QMetaType::Long:
            object[property.name()]  = property.read(this).toInt();
            break;

        case QMetaType::Float:
            object[property.name()]  = property.read(this).toFloat();
            break;

        case QVariant::Type::Double:
            object[property.name()]  = property.read(this).toDouble();
            break;

        case QVariant::Type::String:
            object[property.name()]  = property.read(this).toString();
            break;

        case QVariant::Type::Color: {
            QJsonObject color;
            color["red"]     = this->color().red();
            color["green"]   = this->color().green();
            color["blue"]    = this->color().blue();
            color["alpha"]   = this->color().alpha();
            object[property.name()]  = color;
            break;
        }
        case QVariant::Type::UserType: {
            object[property.name()] = property.read(this).toInt();
        }
        default:
            ;
        }
    }

    return object;
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

void Item::applyData(const QJsonArray &data, const QJsonArray &timeData)
{
    {
        std::lock_guard guard(m_dataMutex);

        if (frequencyDomainSize() != static_cast<unsigned int>(data.count())) {
            setFrequencyDomainSize(static_cast<unsigned int>(data.count()));
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

        if (timeDomainSize() != static_cast<unsigned int>(timeData.count())) {
            setTimeDomainSize(static_cast<unsigned int>(timeData.count()));
        }

        for (int i = 0; i < timeData.count(); i++) {
            auto row = timeData[i].toArray();
            if (row.count() > 0) m_impulseData[i].time   = static_cast<float>(row[0].toDouble());
            if (row.count() > 1) m_impulseData[i].value  = static_cast<float>(row[1].toDouble());

        }
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

void Item::properiesChanged()
{
    auto sender = QObject::sender();
    if (!sender) return;

    auto metaObject = sender->metaObject();
    if (!metaObject) return;

    QString signalName = metaObject->method(QObject::senderSignalIndex()).name();
    QString propertyName = signalName.left(signalName.indexOf("Changed"));


    property("active");
    if (!m_eventSilence && signalName != "stateChanged" & signalName != "activeChanged" ) {
        emit localChanged(propertyName);
    }
}

void Item::setEventSilence(bool eventSilence)
{
    m_eventSilence = eventSilence;
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

void Item::dataError(const uint hash, const bool deactivate)
{
    if (hash != qHash(sourceId())) {
        return;
    }
    auto guard = std::lock_guard<std::mutex>(m_dataMutex);
    setState(Item::ERROR_STATE);
    if (deactivate) {
        setActive(false);
    }

}

void Item::dataReceived(const uint hash, const QJsonArray &data, const QJsonArray &timeData)
{
    if (hash != qHash(sourceId())) {
        return;
    }

    applyData(data, timeData);
}

} // namespace remote
