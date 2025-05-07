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
#include "sourcelist.h"
#include "generator.h"
#include "meta/metabase.h"
#include "remote/server.h"
#include "remote/item.h"

namespace remote {

Server::Server(std::shared_ptr<Generator> generator, QObject *parent) : QObject(parent),
    m_uuid(QUuid::createUuid()), m_networkThread(), m_network(), m_sourceList(nullptr),
    m_generator(generator), m_generatorEnable(false)
{
    m_network.moveToThread(&m_networkThread);
    m_timer.moveToThread(&m_networkThread);
    m_networkThread.setObjectName("NetworkServer");

    connect(&m_networkThread, &QThread::started,  &m_network, &Network::startTCPServer);
    connect(&m_networkThread, &QThread::finished, &m_network, &Network::stopTCPServer);

    m_timer.setInterval(TIMER_INTERVAL);
    connect(&m_timer, &QTimer::timeout, this, &Server::sendHello);
    connect(&m_networkThread, &QThread::started,  &m_timer, [&]() {
        m_timer.start();
    });
    connect(&m_networkThread, &QThread::finished, &m_timer, &QTimer::stop);

    m_network.setTcpCallback([this] (const QHostAddress && address, const QByteArray && data) -> QByteArray {
        return tcpCallback(std::move(address), std::move(data));
    });

    for (int i = 0 ; i < m_generator->metaObject()->propertyCount(); ++i) {
        auto property = m_generator->metaObject()->property(i);
        auto signal = property.notifySignal();
        auto revision = property.revision();
        auto normalizedSignature = QMetaObject::normalizedSignature("sendGeneratorNotify()");
        auto slotId = metaObject()->indexOfMethod(normalizedSignature);
        if (signal.isValid() && revision != NO_API_REVISION) {
            connect(m_generator.get(), signal, this, metaObject()->method(slotId));
        }
    }
}

Server::~Server()
{
    stop();
}

void Server::setSourceList(SourceList *list)
{
    m_sourceList = list;
    if (!m_sourceList) {
        return;
    }
    connectSourceList(list);
}

void Server::connectSourceList(SourceList *list, const Shared::Source &group)
{
    QJsonObject groupJson;
    if (group) {
        groupJson["group"] = group.uuid().toString();
    }

    auto onAdded = [this, group, groupJson](const Shared::Source & source) {

        if (!source || std::dynamic_pointer_cast<remote::Item>(source)) {
            return ;
        }

        sourceNotify(source, "added", groupJson);

        connect(source.get(), &Abstract::Source::readyRead, this, [this, source]() {
            sourceNotify(source, "readyRead");
            sourceNotify(source, "levels", source->levels());
        });

        for (int i = 0 ; i < source->metaObject()->propertyCount(); ++i) {
            auto property = source->metaObject()->property(i);
            auto signal = property.notifySignal();
            auto revision = property.revision();
            auto normalizedSignature = QMetaObject::normalizedSignature("sendSouceNotify()");
            auto slotId = metaObject()->indexOfMethod(normalizedSignature);
            if (signal.isValid() && revision != NO_API_REVISION) {
                connect(source.get(), signal, this, metaObject()->method(slotId));
            }
        }

        if (auto group = std::dynamic_pointer_cast<Source::Group>(source)) {
            connectSourceList(group->sourceList(), source);
        }
    };

    for (const auto &source : *list) {
        onAdded(source);
    }

    connect(list, &SourceList::postItemAppended, this, onAdded);
    connect(list, &SourceList::preItemRemoved, this, [this, list, groupJson](auto uuid) {
        auto source = list->getByUUid(uuid);
        if (source) {
            sourceNotify(source, "removed", groupJson);
            source->disconnect(this);
        }
    });
}

bool Server::generatorEnable() const
{
    return m_generatorEnable;
}

void Server::setGeneratorEnable(bool newGeneratorEnable)
{
    if (m_generatorEnable == newGeneratorEnable)
        return;
    m_generatorEnable = newGeneratorEnable;
    emit generatorEnableChanged();
}

bool Server::start()
{
    m_networkThread.start();
    sendHello();
    emit activeChanged();
    return m_networkThread.isRunning();
}

void Server::stop()
{
    m_network.disconnect();
    m_networkThread.quit();
    m_networkThread.wait();
    emit activeChanged();
}

bool Server::active() const
{
    return m_networkThread.isRunning();
}

void Server::setActive(bool state)
{
    if (active() && !state) {
        stop();
    }
    if (!active() && state) {
        start();
    }
}

QString Server::lastConnected() const
{
    return m_lastConnected;
}

void Server::sendSouceNotify()
{
    auto sender = QObject::sender();
    if (!sender) return;

    auto metaObject = sender->metaObject();
    if (!metaObject) return;

    QString signalName = metaObject->method(QObject::senderSignalIndex()).name();

    auto source = dynamic_cast<Abstract::Source *>(QObject::sender());
    if (source && m_sourceList) {
        auto shared = m_sourceList->getByUUid(source->uuid());
        sourceNotify(shared, "changed");
    }
}

void Server::sendGeneratorNotify()
{
    auto object          = prepareMessage("generator_changed");
    object["generator"]  = m_generator->uuid().toString();
    object["objectName"] = m_generator->objectName();
    object["data"]       = QJsonValue();
    QJsonDocument document(std::move(object));
    sendMulticast(document.toJson(QJsonDocument::JsonFormat::Compact));
}

void Server::setLastConnected(const QString &lastConnected)
{
    if (m_lastConnected != lastConnected) {
        m_lastConnected = lastConnected;
        emit lastConnectedChanged();
    }
}

QByteArray Server::tcpCallback([[maybe_unused]] const QHostAddress &&address, const QByteArray &&data)
{
    auto document = QJsonDocument::fromJson(data);
    if (document.isNull()) {
        return {};
    }

    if (!m_sourceList) {
        return {};
    }
    auto guard = m_sourceList->lock();
    auto message = document["message"].toString();
    auto sourceId = QUuid::fromString(document["uuid"].toString());
    auto source = m_sourceList->getByUUid(sourceId);
    QObject *targetQObject = nullptr;
    if (source) {
        targetQObject = source.get();
    } else if (document["objectName"].toString() == "GeneratorRemote" && sourceId == m_generator->uuid()
               && generatorEnable()) {
        targetQObject = m_generator.get();
    }
    setLastConnected(document["name"].toString());

    if (!targetQObject) {
        QJsonObject object;
        object["api"]     = "Open Sound Meter";
        object["version"] = APP_GIT_VERSION;
        object["message"] = "error";
        object["string"]  = "source not found";
        QJsonDocument document(std::move(object));
        return document.toJson(QJsonDocument::JsonFormat::Compact);
    }

    if (targetQObject && message == "requestChanged") {
        QJsonObject object;
        object["api"]     = "Open Sound Meter";
        object["version"] = APP_GIT_VERSION;
        object["message"] = "sourceSettings";
        object["uuid"]    = sourceId.toString();

        for (int i = 0 ; i < targetQObject->metaObject()->propertyCount(); ++i) {
            auto property = targetQObject->metaObject()->property(i);

            switch (static_cast<int>(property.type())) {

            case QVariant::Type::Bool:
                object[property.name()]  = property.read(targetQObject).toBool();
                break;

            case QVariant::Type::UInt:
            case QVariant::Type::Int:
            case QMetaType::Long:
                object[property.name()]  = property.read(targetQObject).toInt();
                break;

            case QMetaType::Float:
                object[property.name()]  = property.read(targetQObject).toFloat();
                break;

            case QVariant::Type::Double:
                object[property.name()]  = property.read(targetQObject).toDouble();
                break;

            case QVariant::Type::String:
                object[property.name()]  = property.read(targetQObject).toString();
                break;

            case QVariant::Type::Color: {
                QJsonObject color;
                if (source) {
                    color["red"]     = source->color().red();
                    color["green"]   = source->color().green();
                    color["blue"]    = source->color().blue();
                    color["alpha"]   = source->color().alpha();
                }
                object[property.name()]  = color;
                break;
            }
            case QVariant::Type::UserType: {
                object[property.name()] = property.read(targetQObject).toInt();
            }
            default:
                ;
            }
        }

        if (auto group = std::dynamic_pointer_cast<Source::Group>(source)) {
            QJsonArray subSources {};
            for (const auto &subSource : *group->sourceList()) {
                if (source && !std::dynamic_pointer_cast<remote::Item>(subSource)) {
                    QJsonObject subSourceObject;
                    subSourceObject["uuid"] = subSource->uuid().toString();
                    subSourceObject["objectName"] = subSource->objectName();
                    subSources.push_back(subSourceObject);
                }
            }

            object["sources"] = subSources;
        }

        QJsonDocument document(std::move(object));
        return document.toJson(QJsonDocument::JsonFormat::Compact);
    }

    if (targetQObject && message == "update") {
        QJsonObject object;
        object["api"]     = "Open Sound Meter";
        object["version"] = APP_GIT_VERSION;
        object["message"] = "updated";
        object["uuid"]    = sourceId.toString();

        auto itemData = document["data"].toObject();

        auto metaObject = targetQObject->metaObject();
        for (auto &field : itemData.keys()) {

            if (field == "objectName" || field == "active") {
                continue;
            }

            auto index = metaObject->indexOfProperty(field.toStdString().c_str());
            if (index == -1) {
                continue;
            }
            auto property = metaObject->property(index);

            switch (static_cast<int>(property.type())) {
            case QVariant::Type::Bool:
                property.write(targetQObject, itemData[field].toBool());
                break;

            case QVariant::Type::UInt:
            case QVariant::Type::Int:
            case QMetaType::Long:
                property.write(targetQObject, itemData[field].toInt());
                break;


            case QMetaType::Float:
            case QVariant::Type::Double:
                property.write(targetQObject, itemData[field].toDouble());
                break;

            case QVariant::Type::String:
                property.write(targetQObject, itemData[field].toString());
                break;

            case QVariant::Type::Color: {
                auto colorObject = itemData[field].toObject();
                QColor color(
                    colorObject["red"  ].toInt(0),
                    colorObject["green"].toInt(0),
                    colorObject["blue" ].toInt(0),
                    colorObject["alpha"].toInt(1));
                property.write(targetQObject, color);
                break;
            }
            case QVariant::Type::UserType:
                property.write(targetQObject, itemData[field].toInt());
                break;
            default:
                ;
            }
        }
        QJsonDocument document(std::move(object));
        return document.toJson(QJsonDocument::JsonFormat::Compact);
    }

    if (source && message == "requestData") {
        QJsonObject object;
        object["api"]     = "Open Sound Meter";
        object["version"] = APP_GIT_VERSION;
        object["message"] = "sourceData";
        object["uuid"]    = source->uuid().toString();

        QJsonArray ftdata;
        QJsonArray ftcell = {0, 0, 0, 0, 0};
        for (unsigned int i = 0; i < source->frequencyDomainSize(); ++i) {
            ftcell[0] = static_cast<double>(source->frequency(i)  );
            ftcell[1] = static_cast<double>(source->module(i)     );
            ftcell[2] = static_cast<double>(source->magnitudeRaw(i));
            ftcell[3] = static_cast<double>(source->phase(i).arg());
            ftcell[4] = static_cast<double>(source->coherence(i)  );
            //ftcell[5] = static_cast<double>(source->peakSquared(i));
            //ftcell[6] = static_cast<double>(source->meanSquared(i));

            if ( i >= static_cast<unsigned int>(ftdata.size())) {
                ftdata << std::move(ftcell);
            } else {
                ftdata[i] = std::move(ftcell);
            }
        }
        object["ftdata"] = std::move(ftdata);

        QJsonArray timeData;
        QJsonArray timeCell = {0, 0};
        for (unsigned int i = 0; i < source->timeDomainSize(); ++i) {
            timeCell[0] = source->impulseTime(i);
            timeCell[1] = source->impulseValue(i);

            if ( i >= static_cast<unsigned int>(timeData.size())) {
                timeData << std::move(timeCell);
            } else {
                timeData[i] = std::move(timeCell);
            }
        }
        object["timeData"] = std::move(timeData);

        QJsonDocument document(std::move(object));
        return document.toJson(QJsonDocument::JsonFormat::Compact);
    }

    if (source && message == "command") {
        auto itemData = document["data"].toObject();
        auto name = itemData["name"].toString();
        auto argType = itemData["argType"].toString();
        auto argValue = itemData["argValue"].toObject();


        if (!name.isEmpty()) {

            if (name != "store") {

                auto arg = QGenericArgument();
                QVariant argValue;

                if (argType == "float") {
                    argValue = static_cast<float>(itemData["argValue"].toDouble());
                    arg = QGenericArgument("float", argValue.data());
                } else if (!argType.isNull()) {
                    qDebug() << "unused argument";
                }

                QMetaObject::invokeMethod(
                    source.get(),
                    name.toLocal8Bit().data(),
                    Qt::QueuedConnection,
                    arg);
            } else {
                QMetaObject::invokeMethod(
                    m_sourceList,
                    "storeItem",
                    Qt::QueuedConnection,
                    Q_ARG(Shared::Source, source));
            }
        }
    }
    return {};
}

QJsonObject Server::prepareMessage(const QString &message) const
{
    QJsonObject object;
    object["api"] = "Open Sound Meter";
    object["version"] = APP_GIT_VERSION;
    object["host"] = QHostInfo::localHostName();
    object["message"] = message;
    object["uuid"] = m_uuid.toString();
    object["time"] = QTime::currentTime().toString(Qt::ISODate);
    return object;
}

void Server::sourceNotify(const Shared::Source &source, const QString &message, const QJsonValue &data)
{
    if (active() && source) {
        auto object = prepareMessage(message);
        object["source"]     = source->uuid().toString();
        object["objectName"] = source->objectName();
        object["data"]       = data;
        QJsonDocument document(std::move(object));
        sendMulticast(document.toJson(QJsonDocument::JsonFormat::Compact));
    }
}

void Server::sendMulticast(const QByteArray &data)
{
    QMetaObject::invokeMethod(
        &m_network,
        "sendUDP",
        Qt::QueuedConnection,
        Q_ARG(QByteArray, std::move(data)),
        Q_ARG(QString, m_network.MULTICAST_IP));
}

void Server::sendHello()
{
    auto object = prepareMessage("hello");
    object["port"] = m_network.port();
    object["multicast"] = m_network.MULTICAST_IP;
    QJsonArray sources {};
    if (m_sourceList) {
        for (const auto &source : *m_sourceList) {
            if (source && !std::dynamic_pointer_cast<remote::Item>(source)) {
                QJsonObject sourceObject;
                sourceObject["uuid"] = source->uuid().toString();
                sourceObject["objectName"] = source->objectName();
                sources.push_back(sourceObject);
            }
        }
    }

    object["sources"] = sources;
    object["generator"] = (generatorEnable() ? m_generator->uuid().toString() : "disable");
    QJsonDocument document(std::move(object));
    QByteArray data = document.toJson(QJsonDocument::JsonFormat::Compact);
    QMetaObject::invokeMethod(
        &m_network,
        "sendUDP",
        Qt::QueuedConnection,
        Q_ARG(QByteArray, std::move(data))
    );
}

} // namespace remote
