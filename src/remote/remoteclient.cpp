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
#include "remoteclient.h"
#include "sourcelist.h"
#include "item.h"
#include "remote/items/storeditem.h"
#include "remote/items/measurementitem.h"
#include "remote/items/groupitem.h"

namespace remote {

Client::Client(Settings *settings, QObject *parent) : QObject(parent),
    m_network(),
    m_settings(settings),
    m_thread(), m_timer(),
    m_sourceList(nullptr), m_servers(), m_items(), m_onRequest(false), m_updateCounter(0), m_needUpdate()
{
    connect(&m_network, &Network::datagramRecieved, this, &Client::processData);
    m_thread.setObjectName("NetworkClient");
    m_network.moveToThread(&m_thread);
    m_timer.setInterval(TIMER_INTERVAL);
    m_timer.moveToThread(&m_thread);

    connect(this, &Client::newRemoteItem, this, &Client::appendItem, Qt::QueuedConnection);
    connect(&m_timer, &QTimer::timeout, this, &Client::sendRequests, Qt::DirectConnection);
    connect(&m_thread, &QThread::started, this, [this]() {
        m_timer.start();
    }, Qt::DirectConnection);
    connect(&m_thread, &QThread::finished, this, [this]() {
        m_timer.stop();
    }, Qt::DirectConnection);
}

Client::~Client()
{
    stop();
}

void Client::setSourceList(SourceList *list)
{
    m_sourceList = list;
}

bool Client::start()
{
    QMetaObject::invokeMethod(&m_network, "bindUDP");
    m_thread.start();
    return m_thread.isRunning();
}

void Client::stop()
{
    QMetaObject::invokeMethod(&m_network, "unbindUDP");
    m_thread.quit();
    m_thread.wait();
}

bool Client::active() const
{
    return m_thread.isRunning();
}

void Client::setActive(bool state)
{
    if (active() && !state) {
        stop();
    }
    if (!active() && state) {
        start();
    }
}

void Client::reset()
{
    auto restart = m_thread.isRunning();
    stop();

    for (auto &item : m_items) {
        m_sourceList->removeItem(std::static_pointer_cast<Abstract::Source>(item), true);
    }
    m_items.clear();

    if (restart) {
        start();
    }
}

QStringList Client::generatorsList() const
{
    QStringList list;
    list << "Local";
    for (auto &&generator : m_generators) {
        if (generator) {
            list << generator->host();
        }
    }
    return list;
}

void Client::selectGenerator(const QString &name)
{
    for (auto &&generator : m_generators) {
        if (generator && (generator->host() == name)) {
            setControlledGenerator( generator );
            return;
        }
    }
    setControlledGenerator({});
}

void Client::sendRequests()
{
    if (m_onRequest) {
        return;
    }
    auto guard = m_sourceList->lock();
    std::shared_ptr<Item> updateItem;
    UpdateKey minUpdate = READY_FOR_UPDATE;
    unsigned int minUpdateKey = 0;

    for (auto &key : m_needUpdate.keys()) {
        auto item = m_items.value(key, nullptr);
        auto updateValue = m_needUpdate.value(key, READY_FOR_UPDATE);

        if (item && item->active() && updateValue < minUpdate) {
            minUpdateKey = key;
            minUpdate = updateValue;
            updateItem = item;
        }
    }

    if (updateItem) {
        m_needUpdate[minUpdateKey] = ON_UPDATE;
        m_onRequest = true;
        requestData(updateItem);
    }
}

void Client::sendCommand(const std::shared_ptr<Item> &item, QString command, QVariant arg)
{
    Network::responseCallback onAnswer = [](const QByteArray &) {};

    if (item) {
        QJsonObject object;
        object["name"] = command;

        switch (arg.type()) {
        case QVariant::Type::Invalid:
            break;
        case QMetaType::Float:
        case QVariant::Type::Double:
            object["argType"] = "float";
            object["argValue"] = arg.toFloat();
            break;

        default:
            qDebug() << "unknown type: " << arg.typeName();
        }
        requestSource(item, "command", onAnswer, {}, object);
    }
}

void Client::appendItem(const QUuid &serverId, const QUuid &sourceId, const QString &objectName, const QString &host,
                        const QUuid groupId)
{
    auto item = addItem(serverId, sourceId, objectName, host, groupId);
    requestChanged(item);
    requestUpdate(item);
}

std::shared_ptr<Item> Client::addItem(const QUuid &serverId, const QUuid &sourceId, const QString &objectName,
                                      const QString &host, const QUuid groupId)
{
    std::shared_ptr<Item> item;
    if (objectName == "Measurement") {
        item = std::make_shared<remote::MeasurementItem>(this);
    } else if (objectName == "Stored") {
        item = std::make_shared<remote::StoredItem>(this);
    } else if (objectName == "Group") {
        item = std::make_shared<remote::GroupItem>(this);
    } else {
        item = std::make_shared<remote::Item>(this);
    }
    item->setServerId(serverId);
    item->setSourceId(sourceId);
    item->setHost(host);

    connect(this, &Client::dataError, item.get(), &Item::dataError);
    connect(this, &Client::dataReceived, item.get(), &Item::dataReceived);

    connect(item.get(), &Item::updateData,   this, [ = ]() {
        requestUpdate( item );
    });
    connect(item.get(), &Item::localChanged, this, [ = ](QString propertyName) {
        sendUpdate(item, propertyName);
    });
    connect(item.get(), &Item::sendCommand,  this, [ = ](QString name, QVariant arg) {
        sendCommand(item, name, arg);
    });
    connect(item.get(), &Item::beforeDestroy, this, [ = ]() {
        m_items[qHash(sourceId)] = nullptr;
        m_needUpdate[qHash(sourceId)] = READY_FOR_UPDATE;
    }, Qt::DirectConnection);

    item->connectProperties();
    SourceList *targetSource = m_sourceList;
    if (!groupId.isNull()) {
        auto groupItem = std::dynamic_pointer_cast<remote::GroupItem>(m_items.value(qHash(groupId), nullptr));
        if (groupItem) {
            targetSource = groupItem->sourceList();
        }
    }
    targetSource->appendItem(Shared::Source{ item });
    m_items[qHash(sourceId)] = item;

    return item;
}

template <typename ItemType>
void Client::sendUpdate(const std::shared_ptr<ItemType> &item, QString propertyName)
{
    Network::responseCallback onAnswer = [](const QByteArray &) {};

    if (item) {
        requestSource(item, "update", onAnswer, {}, item->metaJsonObject(propertyName));
    }
}

void Client::processData(QHostAddress senderAddress, [[maybe_unused]] int senderPort, const QByteArray &data)
{
    auto document = QJsonDocument::fromJson(data);
    if (document.isNull()) {
        return;
    }
    auto serverId = QUuid::fromString(document["uuid"].toString());
    auto time = document["time"].toString();
    auto host = document["host"].toString();
    Q_UNUSED(time)

    if (document["message"].toString() == "hello") {
        auto port = document["port"].toInt();
        m_servers[qHash(serverId)] = {senderAddress, port};

        if (document["sources"].isArray()) {
            auto sources = document["sources"].toArray();
            for (int i = 0; i < sources.count(); i++) {

                auto sourceObject = sources[i].toObject();
                auto sourceUuid = QUuid::fromString(sourceObject["uuid"].toString());
                auto sourceObjectName = sourceObject["objectName"].toString();

                if (m_items.find(qHash(sourceUuid)) == m_items.end()) {
                    appendItem(serverId, sourceUuid, sourceObjectName, host);
                }
            }
        }

        auto generatorUuid = QUuid::fromString(document["generator"].toString());
        if (!generatorUuid.isNull()) {
            if (m_generators.find(qHash(serverId)) == m_generators.end() ) {
                auto generator = std::make_shared<GeneratorRemote>(this);
                generator->setServerId(serverId);
                generator->setSourceId(generatorUuid);
                generator->setHost(host);

                connect(this, &Client::dataError,    generator.get(), &GeneratorRemote::dataError);

                connect(generator.get(), &GeneratorRemote::localChanged, this, [ = ](QString propertyName) {
                    sendUpdate(generator, propertyName);
                });

                generator->connectProperties();
                m_generators[qHash(serverId)] = generator;
                emit generatorsListChanged();
            } else {
                m_generators[qHash(serverId)]->setState(GeneratorRemote::AVAILABLE);
            }
        } else if (m_generators.find(qHash(serverId)) != m_generators.end() ) {
            m_generators.remove(qHash(serverId));
            emit generatorsListChanged();
        }

        return;
    }

    if (!document["source"].toString().isNull()) {
        auto sourceId = QUuid::fromString(document["source"].toString());
        auto message = document["message"].toString();
        auto item = m_items.value(qHash(sourceId), nullptr);
        if (!m_sourceList) {
            return;
        }

        if (message == "added" && !item) {
            auto data = document["data"].toObject();
            auto groupUuid = data["group"].toString();
            item = addItem(serverId, sourceId, document["objectName"].toString(), host, groupUuid);
            requestChanged(item);
            requestUpdate(item);
        }

        if (item && message == "removed") {
            m_sourceList->removeItem(item->uuid());
            m_items[qHash(sourceId)] = nullptr;
            m_needUpdate[qHash(sourceId)] = READY_FOR_UPDATE;
        }

        if (item && message == "changed") {
            requestChanged(item);
        }

        if (item && message == "readyRead") {
            requestUpdate(item);
        }

        if (item && message == "levels") {
            item->setLevels(document["data"].toObject());
        }
        return;
    }

    if (document["message"].toString() == "generator_changed") {
        if (m_generators.find(qHash(serverId)) != m_generators.end() ) {
            requestGenearatorChanged(m_generators[qHash(serverId)]);
        }
    }
}

void Client::requestUpdate(const std::shared_ptr<Item> &item)
{
    if (item && item->active()) {
        if (m_needUpdate[qHash(item->sourceId())] == READY_FOR_UPDATE) {
            m_needUpdate[qHash(item->sourceId())] = ++m_updateCounter;
        }
    }
}

void Client::requestChanged(const std::shared_ptr<Item> &item)
{
    Network::responseCallback onAnswer = [item, this](const QByteArray & data) {
        auto document = QJsonDocument::fromJson(data).object();
        item->setEventSilence(true);
        item->setOriginalActive(document["active"].toBool());

        auto metaObject = item->metaObject();
        for (auto &field : document.keys()) {

            if (field == "objectName" || field == "active") {
                continue;
            }

            if (field == "sources") {
                if (auto groupItem = std::dynamic_pointer_cast<GroupItem>(item)) {


                    auto sources = document["sources"].toArray();
                    for (int i = 0; i < sources.count(); i++) {
                        auto sourceObject = sources[i].toObject();
                        auto sourceUuid = QUuid::fromString(sourceObject["uuid"].toString());
                        auto sourceObjectName = sourceObject["objectName"].toString();
                        if (m_items.find(qHash(sourceUuid)) == m_items.end()) {
                            emit newRemoteItem(groupItem->serverId(), sourceUuid, sourceObjectName, groupItem->host(), groupItem->sourceId());
                        }
                    }
                }
            }

            auto index = metaObject->indexOfProperty(field.toStdString().c_str());
            if (index == -1) {
                continue;
            }
            auto property = metaObject->property(index);

            switch (static_cast<int>(property.type())) {
            case QVariant::Type::Bool:
                property.write(item.get(), document[field].toBool());
                break;

            case QVariant::Type::UInt:
            case QVariant::Type::Int:
            case QMetaType::Long:
                property.write(item.get(), document[field].toInt());
                break;


            case QMetaType::Float:
            case QVariant::Type::Double:
                property.write(item.get(), document[field].toDouble());
                break;

            case QVariant::Type::String:
                property.write(item.get(), document[field].toString());
                break;

            case QVariant::Type::Color: {
                auto colorObject = document[field].toObject();
                QColor color(
                    colorObject["red"  ].toInt(0),
                    colorObject["green"].toInt(0),
                    colorObject["blue" ].toInt(0),
                    colorObject["alpha"].toInt(1));
                property.write(item.get(), color);
                break;
            }
            case QVariant::Type::UserType: {
                property.write(item.get(), document[field].toInt());
                break;
            }
            default:
                ;
            }
        }

        item->setEventSilence(false);
    };
    requestSource(item, "requestChanged", onAnswer, {});
}

void Client::requestGenearatorChanged(const SharedGeneratorRemote &genearator)
{
    Network::responseCallback onAnswer = [genearator, this](const QByteArray & data) {
        auto document = QJsonDocument::fromJson(data).object();
        genearator->setEventSilence(true);

        auto metaObject = genearator->metaObject();
        for (auto &field : document.keys()) {

            if (field == "objectName") {
                continue;
            }

            auto index = metaObject->indexOfProperty(field.toStdString().c_str());
            if (index == -1) {
                continue;
            }
            auto property = metaObject->property(index);

            switch (static_cast<int>(property.type())) {
            case QVariant::Type::Bool:
                property.write(genearator.get(), document[field].toBool());
                break;

            case QVariant::Type::UInt:
            case QVariant::Type::Int:
            case QMetaType::Long:
                property.write(genearator.get(), document[field].toInt());
                break;


            case QMetaType::Float:
            case QVariant::Type::Double:
                property.write(genearator.get(), document[field].toDouble());
                break;

            case QVariant::Type::String:
                property.write(genearator.get(), document[field].toString());
                break;

            case QVariant::Type::Color: {
                auto colorObject = document[field].toObject();
                QColor color(
                    colorObject["red"  ].toInt(0),
                    colorObject["green"].toInt(0),
                    colorObject["blue" ].toInt(0),
                    colorObject["alpha"].toInt(1));
                property.write(genearator.get(), color);
                break;
            }
            case QVariant::Type::UserType: {
                property.write(genearator.get(), document[field].toInt());
                break;
            }
            default:
                ;
            }
        }

        genearator->setEventSilence(false);
    };
    requestSource(genearator, "requestChanged", onAnswer, {});
}

void Client::requestData(const std::shared_ptr<Item> &item)
{
    if (!item) {
        return;
    }
    auto hash = qHash(item->sourceId());
    Network::responseCallback onAnswer = [this, hash](const QByteArray & data) {
        auto document = QJsonDocument::fromJson(data);
        if (!document.isNull()) {
            auto frequencyData = document["ftdata"].toArray();
            auto timeData = document["timeData"].toArray();
            emit dataReceived(hash, frequencyData, timeData);
            m_needUpdate[hash] = READY_FOR_UPDATE;
        } else {
            emit dataError(hash, false);
        }
        m_onRequest = false;
    };
    Network::errorCallback onError = [this, hash]() {

        emit dataError(hash, false);
        qDebug() << "requestData error";
        m_needUpdate[hash] = READY_FOR_UPDATE;
        m_onRequest = false;
    };
    requestSource(item, "requestData", onAnswer, onError);
}

template <typename ItemType>
void Client::requestSource(const std::shared_ptr<ItemType> &item, const QString &message,
                           Network::responseCallback callback,
                           Network::errorCallback errorCallback, QJsonObject itemData)
{
    auto server = m_servers.value(qHash(item->serverId()), {{}, 0});
    if (server.first.isNull()) {
        return;
    }

    QJsonObject object;
    object["name"] = "Open Sound Meter";
    object["version"] = APP_GIT_VERSION;
    object["message"] = message;
    object["uuid"] = item->sourceId().toString();
    object["objectName"] = item->objectName();
    object["data"] = itemData;

    QJsonDocument document(std::move(object));
    auto data = document.toJson(QJsonDocument::JsonFormat::Compact);
    auto hash = qHash(item->sourceId());
    Network::errorCallback onError = [this, hash]() {
        qDebug() << "onError" << Qt::endl;
        emit dataError(hash, true);
    };

    Network::responseErrorCallbacks callbacks = {item, callback, errorCallback ? errorCallback : onError};
    QMetaObject::invokeMethod(
        &m_network,
        "sendTCP",
        Qt::QueuedConnection,
        Q_ARG(QByteArray, std::move(data)),
        Q_ARG(QString, server.first.toString()),
        Q_ARG(quint16, server.second),
        Q_ARG(remote::Network::responseErrorCallbacks, callbacks)
    );
}

SharedGeneratorRemote Client::controlledGenerator() const
{
    return m_controlledGenerator;
}

void Client::setControlledGenerator(const SharedGeneratorRemote &newControlledGenerator)
{
    if (m_controlledGenerator == newControlledGenerator)
        return;
    m_controlledGenerator = newControlledGenerator;
    emit controlledGeneratorChanged();
}

} // namespace remote
