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
#include "items/storeditem.h"
#include "items/measurementitem.h"


#if defined APP_API_KEY
#   define USE_API_KEY APP_API_KEY
#else
#   define USE_API_KEY
#endif

namespace remote {

const QString Client::SETTINGS_LICENSE_KEY = "licenseFile";

Client::Client(Settings *settings, QObject *parent) : QObject(parent), m_network(),  m_key(USE_API_KEY),
    m_settings(settings),
    m_thread(), m_timer(),
    m_sourceList(nullptr), m_servers(), m_items(), m_onRequest(false), m_updateCounter(0), m_needUpdate()
{
    connect(&m_network, &Network::datagramRecieved, this, &Client::dataRecieved);
    m_thread.setObjectName("NetworkClient");
    m_network.moveToThread(&m_thread);
    m_timer.setInterval(TIMER_INTERVAL);
    m_timer.moveToThread(&m_thread);

    connect(&m_timer, &QTimer::timeout, this, &Client::sendRequests, Qt::DirectConnection);
    connect(&m_thread, &QThread::started, this, [this]() {
        m_timer.start();
    }, Qt::DirectConnection);
    connect(&m_thread, &QThread::finished, this, [this]() {
        m_timer.stop();
    }, Qt::DirectConnection);

    if (m_key.isEmpty() || !m_key.valid()) {
        auto savedUrl = m_settings->value(SETTINGS_LICENSE_KEY).toUrl();
        if (savedUrl.isValid()) {
            openLicenseFile(savedUrl);
        }
    }
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
    if (!licensed()) {
        return false;
    }

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

bool Client::openLicenseFile(const QUrl &fileName)
{
    auto savedUrl = m_settings->value(SETTINGS_LICENSE_KEY).toUrl();
    if (savedUrl != fileName) {
        m_settings->setValue(SETTINGS_LICENSE_KEY, fileName);
    }

    QFile loadFile(fileName.toLocalFile());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file");
        return false;
    }
    m_key = {loadFile.readAll()};

    emit licenseChanged();

    return m_key.valid();
}

bool Client::licensed() const
{
    return m_key.valid();
}

QString Client::licenseOwner() const
{
    return m_key.title();
}

bool Client::staticLicense() const
{
#ifdef APP_API_KEY
    return true;
#else
    return false;
#endif
}

void Client::sendRequests()
{
    if (m_onRequest) {
        return;
    }
    auto guard = m_sourceList->lock();
    auto result = std::min_element(m_needUpdate.begin(), m_needUpdate.end());

    if (result != m_needUpdate.end() && (*result) < ON_UPDATE) {
        (*result) = ON_UPDATE;
        m_onRequest = true;
        requestData(m_items[result.key()]);
    }
}

void Client::sendCommand(Item *item, QString command)
{
    Network::responseCallback onAnswer = [](const QByteArray &) {};

    if (item) {
        QJsonObject object;
        object["name"] = command;
        requestSource(item, "command", onAnswer, {}, object);
    }
}

Item *Client::addItem(const QUuid &serverId, const QUuid &sourceId, const QString &objectName, const QString &host)
{
    remote::Item *item;
    if (objectName == "Measurement") {
        item = new remote::MeasurementItem(this);
    } else if (objectName == "Stored") {
        item = new remote::StoredItem(this);
    } else {
        item = new remote::Item(this);
    }
    item->setServerId(serverId);
    item->setSourceId(sourceId);
    item->setHost(host);
    connect(item, &Item::updateData, this, &Client::requestUpdate);
    connect(item, &Item::localChanged, this, [ = ](QString propertyName) {
        sendUpdate(item, propertyName);
    });
    connect(item, &Item::sendCommand, this, [ = ](QString name) {
        sendCommand(item, name);
    });
    connect(item, &Item::destroyed, this, [ = ]() {
        m_items[qHash(sourceId)] = nullptr;
        m_needUpdate[qHash(sourceId)] = READY_FOR_UPDATE;
    }, Qt::DirectConnection);

    item->connectProperties();
    m_sourceList->appendItem(item);
    m_items[qHash(sourceId)] = item;

    return item;
}

void Client::sendUpdate(Item *item, QString propertyName)
{
    Network::responseCallback onAnswer = [](const QByteArray &) {};

    if (item) {
        requestSource(item, "update", onAnswer, {}, item->metaJsonObject(propertyName));
    }
}

void Client::dataRecieved(QHostAddress senderAddress, [[maybe_unused]] int senderPort, const QByteArray &data)
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
                    auto item = addItem(serverId, sourceUuid, sourceObjectName, host);
                    requestChanged(item);
                    requestUpdate(item);
                }
            }
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
            item = addItem(serverId, sourceId, document["objectName"].toString(), host);
            requestChanged(item);
            requestUpdate(item);
        }

        if (item && message == "removed") {
            m_sourceList->removeItem(item);
            m_items[qHash(sourceId)] = nullptr;
            m_needUpdate[qHash(sourceId)] = READY_FOR_UPDATE;
        }

        if (item && message == "changed") {
            requestChanged(item);
        }

        if (item && message == "readyRead") {
            requestUpdate(item);
        }
        return;
    }
}

void Client::requestUpdate(Item *item)
{
    if (item && item->active()) {
        if (m_needUpdate[qHash(item->sourceId())] == READY_FOR_UPDATE) {
            m_needUpdate[qHash(item->sourceId())] = ++m_updateCounter;
        }
    }
}

void Client::requestChanged(Item *item)
{
    Network::responseCallback onAnswer = [item](const QByteArray & data) {
        auto document = QJsonDocument::fromJson(data).object();
        item->setEventSilence(true);
        item->setOriginalActive(document["active"].toBool());

        auto metaObject = item->metaObject();
        for (auto &field : document.keys()) {

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
                property.write(item, document[field].toBool());
                break;

            case QVariant::Type::UInt:
            case QVariant::Type::Int:
            case QMetaType::Long:
                property.write(item, document[field].toInt());
                break;


            case QMetaType::Float:
            case QVariant::Type::Double:
                property.write(item, document[field].toDouble());
                break;

            case QVariant::Type::String:
                property.write(item, document[field].toString());
                break;

            case QVariant::Type::Color: {
                auto colorObject = document[field].toObject();
                QColor color(
                    colorObject["red"  ].toInt(0),
                    colorObject["green"].toInt(0),
                    colorObject["blue" ].toInt(0),
                    colorObject["alpha"].toInt(1));
                property.write(item, color);
                break;
            }
            case QVariant::Type::UserType: {
                property.write(item, document[field].toInt());
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

void Client::requestData(Item *item)
{
    if (!item) {
        return;
    }
    Network::responseCallback onAnswer = [this, item](const QByteArray & data) {
        auto document = QJsonDocument::fromJson(data);
        auto ftData = document["ftdata"].toArray();
        item->applyData(ftData);
        m_needUpdate[qHash(item->sourceId())] = READY_FOR_UPDATE;
        m_onRequest = false;
    };
    Network::errorCallback onError = [this, item]() {
        item->setState(Item::ERROR);
        qDebug() << "requestData error";
        m_needUpdate[qHash(item->sourceId())] = READY_FOR_UPDATE;
        m_onRequest = false;
    };
    requestSource(item, "requestData", onAnswer, onError);
}

void Client::requestSource(Item *item, const QString &message, Network::responseCallback callback,
                           Network::errorCallback errorCallback, QJsonObject itemData)
{
    auto server = m_servers.value(qHash(item->serverId()), {{}, 0});
    if (server.first.isNull()) {
        return;
    }

    QJsonObject object;
    object["api"] = "Open Sound Meter";
    object["version"] = APP_GIT_VERSION;
    object["message"] = message;
    object["uuid"] = item->sourceId().toString();

    QJsonObject license;
    license["owner"] = m_key.owner();
    license["type"] = m_key.type();
    license["sign"] = m_key.sign();
    object["license"] = license;
    object["data"] = itemData;

    QJsonDocument document(std::move(object));
    auto data = document.toJson(QJsonDocument::JsonFormat::Compact);

    Network::errorCallback onError = [item]() {
        qDebug() << "onError";
        item->setState(Item::ERROR);
        item->setActive(false);
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

} // namespace remote
