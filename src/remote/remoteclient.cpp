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

void Client::dataRecieved(QHostAddress senderAddress, int senderPort, const QByteArray &data)
{
    auto document = QJsonDocument::fromJson(data);
    if (document.isNull()) {
        return;
    }

    if (document["message"].toString() == "hello") {
        auto uuid = QUuid::fromString(document["uuid"].toString());
        m_servers[qHash(uuid)] = {senderAddress, senderPort};
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

        if (message == "changed") {
            //update source by TCP
        }

        if (message == "readyRead") {
            //schedulre TCP data request
        }

        return;
    }
}

} // namespace remote
