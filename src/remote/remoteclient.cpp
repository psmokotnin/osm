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

namespace remote {

Client::Client(QObject *parent) : QObject(parent), m_network(), m_sourceList(nullptr), m_servers(), m_items()
{
    connect(&m_network, &Network::datagramRecieved, this, &Client::dataRecieved);
    m_network.bindUDP();
    m_network.joinMulticast();
}

void Client::setSourceList(SourceList *list)
{
    m_sourceList = list;
}

void Client::dataRecieved(QHostAddress senderAddress, [[maybe_unused]] int senderPort, const QByteArray &data)
{
    auto document = QJsonDocument::fromJson(data);
    if (document.isNull()) {
        return;
    }

    if (document["message"].toString() == "hello") {
        auto uuid = QUuid::fromString(document["uuid"].toString());
        auto port = document["port"].toInt();

        m_servers[qHash(uuid)] = {senderAddress, port};
        return;
    }

    if (!document["source"].toString().isNull()) {
        auto sourceId = QUuid::fromString(document["source"].toString());
        auto serverId = QUuid::fromString(document["uuid"].toString());
        auto message = document["message"].toString();
        auto item = m_items.value(qHash(sourceId), nullptr);

        if (!m_sourceList) {
            return;
        }

        if (message == "added" && !item) {
            item = new remote::Item(this);
            item->setServerId(serverId);
            item->setSourceId(sourceId);
            m_sourceList->appendItem(item);
            m_items[qHash(sourceId)] = item;

            message = "changed";
        }

        if (item && message == "removed") {
            m_sourceList->removeItem(item);
        }

        if (item && message == "changed") {
            requestChanged(item);
        }

        if (item && message == "readyRead") {
            requestData(item);
        }

        return;
    }
}

void Client::requestChanged(Item *item)
{
    Network::responseCallback onAnswer = [item](const QByteArray & data) {
        auto document = QJsonDocument::fromJson(data);
        auto jsonColor = document["color"].toObject();
        QColor c(
            jsonColor["red"  ].toInt(0),
            jsonColor["green"].toInt(0),
            jsonColor["blue" ].toInt(0),
            jsonColor["alpha"].toInt(1));
        item->setColor(c);
        item->setName(document["name"].toString());
        item->setOriginalActive(document["active"].toBool());
    };
    requestSource(item, "requestChanged", onAnswer);
}

void Client::requestData(Item *item)
{
    Network::responseCallback onAnswer = [item](const QByteArray & data) {
        auto document = QJsonDocument::fromJson(data);
        auto ftData = document["ftdata"].toArray();
        item->applyData(ftData);
    };
    requestSource(item, "requestData", onAnswer);
}

void Client::requestSource(Item *item, const QString &message, Network::responseCallback callback)
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
    QJsonDocument document(object);
    auto data = document.toJson(QJsonDocument::JsonFormat::Compact);

    Network::errorCallback onError = [item]() {
        qDebug() << "onError";
        item->setActive(false);
    };
    m_network.sendTCP(data, server.first.toString(), server.second, callback, onError);
}

} // namespace remote
