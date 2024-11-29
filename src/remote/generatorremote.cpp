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
#include "generatorremote.h"
#include <QJsonObject>
#include <QMetaProperty>

namespace remote {

GeneratorRemote::GeneratorRemote(QObject *parent) : QObject(parent),
    m_enabled(false),
    m_type(0),
    m_frequency(1000),
    m_startFrequency(20),
    m_endFrequency(20000),
    m_gain(-6.f), m_duration(1.f),
    m_eventSilence(false),
    m_sourceId("generator"),
    m_state(NOT_AVAILABLE),
    m_stateTimer()
{
    setObjectName("GeneratorRemote");

    m_stateTimer.setSingleShot(true);
    m_stateTimer.setInterval(1000);
    connect(&m_stateTimer, &QTimer::timeout, this, &GeneratorRemote::resetState);
    connect(this, &GeneratorRemote::stateChanged, this, &GeneratorRemote::startResetTimer);
    connect(this, &GeneratorRemote::stateChanged, this, &GeneratorRemote::startResetTimer);
}

bool GeneratorRemote::enabled() const
{
    return m_enabled;
}

void GeneratorRemote::setEnabled(bool newEnabled)
{
    if (m_enabled == newEnabled)
        return;
    m_enabled = newEnabled;
    emit enabledChanged();
}

int GeneratorRemote::type() const
{
    return m_type;
}

void GeneratorRemote::setType(int newType)
{
    if (m_type == newType)
        return;
    m_type = newType;
    emit typeChanged();
}

int GeneratorRemote::frequency() const
{
    return m_frequency;
}

void GeneratorRemote::setFrequency(int newFrequency)
{
    if (m_frequency == newFrequency)
        return;
    m_frequency = newFrequency;
    emit frequencyChanged();
}

int GeneratorRemote::startFrequency() const
{
    return m_startFrequency;
}

void GeneratorRemote::setStartFrequency(int newStartFrequency)
{
    if (m_startFrequency == newStartFrequency)
        return;
    m_startFrequency = newStartFrequency;
    emit startFrequencyChanged();
}

int GeneratorRemote::endFrequency() const
{
    return m_endFrequency;
}

void GeneratorRemote::setEndFrequency(int newEndFrequency)
{
    if (m_endFrequency == newEndFrequency)
        return;
    m_endFrequency = newEndFrequency;
    emit endFrequencyChanged();
}

float GeneratorRemote::gain() const
{
    return m_gain;
}

void GeneratorRemote::setGain(float newGain)
{
    if (qFuzzyCompare(m_gain, newGain))
        return;
    m_gain = newGain;
    emit gainChanged();
}

float GeneratorRemote::duration() const
{
    return m_duration;
}

void GeneratorRemote::setDuration(float newDuration)
{
    if (qFuzzyCompare(m_duration, newDuration))
        return;
    m_duration = newDuration;
    emit durationChanged();
}

QUuid GeneratorRemote::serverId() const
{
    return m_serverId;
}

void GeneratorRemote::setServerId(const QUuid &newServerId)
{
    m_serverId = newServerId;
}

QString GeneratorRemote::host() const
{
    return m_host;
}

void GeneratorRemote::setHost(const QString &newHost)
{
    if (m_host == newHost)
        return;
    m_host = newHost;
    emit hostChanged();
}

GeneratorRemote::State GeneratorRemote::state() const
{
    return m_state;
}

void GeneratorRemote::setState(const GeneratorRemote::State &newState)
{
    if (m_state == newState)
        return;
    m_state = newState;
    emit stateChanged();
}

QJsonObject GeneratorRemote::metaJsonObject(QString propertyName) const
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

        case QVariant::Type::UserType: {
            object[property.name()] = property.read(this).toInt();
        }
        default:
            ;
        }
    }

    return object;
}

void GeneratorRemote::startResetTimer()
{
    if (m_state == AVAILABLE) {
        m_stateTimer.start();
    }
}

void GeneratorRemote::resetState()
{
    if (m_state == AVAILABLE) {
        setState(NOT_AVAILABLE);
    }
}

void GeneratorRemote::connectProperties()
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

void GeneratorRemote::dataError(const uint hash, const bool deactivate)
{
    if (hash != qHash(serverId())) {
        return;
    }
    setState(GeneratorRemote::ERROR_STATE);
}

void GeneratorRemote::properiesChanged()
{
    auto sender = QObject::sender();
    if (!sender) return;

    auto metaObject = sender->metaObject();
    if (!metaObject) return;

    QString signalName = metaObject->method(QObject::senderSignalIndex()).name();
    QString propertyName = signalName.left(signalName.indexOf("Changed"));

    if ( !m_eventSilence && signalName != "stateChanged" ) {
        emit localChanged(propertyName);
    }
}

QUuid GeneratorRemote::sourceId() const
{
    return m_sourceId;
}

void GeneratorRemote::setSourceId(const QUuid &newSourceId)
{
    m_sourceId = newSourceId;
}

void GeneratorRemote::setEventSilence(bool eventSilence)
{
    m_eventSilence = eventSilence;
}


} // namespace remote

SharedGeneratorRemote::SharedGeneratorRemote(std::shared_ptr<remote::GeneratorRemote> ptr)
    : std::shared_ptr<remote::GeneratorRemote>(ptr)
{

}

SharedGeneratorRemote::~SharedGeneratorRemote() = default;
