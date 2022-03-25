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
#include "server.h"
#include "../sourcelist.h"
#include "item.h"

namespace remote {

Server::Server(QObject *parent) : QObject(parent),
    m_uuid(QUuid::createUuid()), m_networkThread(), m_network(), m_sourceList(nullptr)
{
    m_network.moveToThread(&m_networkThread);
    m_networkThread.setObjectName("Network");

    connect(&m_networkThread, &QThread::started,  &m_network, &Network::startTCPServer);
    connect(&m_networkThread, &QThread::finished, &m_network, &Network::stopTCPServer);

    m_timer.setInterval(TIMER_INTERVAL);
    connect(&m_timer, &QTimer::timeout, this, &Server::sendHello);
    connect(&m_networkThread, &QThread::started,  this, &Server::sendHello);
}

Server::~Server()
{
    stop();
}

void Server::setSourceList(SourceList *list)
{
    m_sourceList = list;
    auto onAdded = [this](auto * source) {

        if (!source || dynamic_cast<remote::Item *>(source)) {
            return ;
        }

        sourceNotify(source->uuid(), "added");

        connect(source, &chart::Source::readyRead, [this, source]() {
            sourceNotify(source->uuid(), "readyRead");
        });

        connect(source, &chart::Source::activeChanged, [this, source]() {
            sourceNotify(source->uuid(), "changed");
        });

        connect(source, &chart::Source::colorChanged, [this, source]() {
            sourceNotify(source->uuid(), "changed");
        });

        connect(source, &chart::Source::nameChanged, [this, source]() {
            sourceNotify(source->uuid(), "changed");
        });
    };

    for (auto *source : *list) {
        onAdded(source);
    }

    connect(list, &SourceList::postItemAppended, onAdded);
    connect(list, &SourceList::preItemRemoved, [this, list](auto index) {
        auto *source = list->get(index);
        if (source) {
            sourceNotify(source->uuid(), "removed");
        }
    });
}

bool Server::start()
{
    m_networkThread.start();
    m_timer.start();
    return m_networkThread.isRunning();
}

void Server::stop()
{
    m_network.disconnect();
    m_networkThread.quit();
    m_networkThread.wait();
}

QJsonObject Server::prepareMessage(const QString &message) const
{
    QJsonObject object;
    object["api"] = "Open Sound Meter";
    object["version"] = APP_GIT_VERSION;
    object["message"] = message;
    object["uuid"] = m_uuid.toString();
    return object;
}

void Server::sourceNotify(const QUuid &id, const QString &message)
{
    auto object = prepareMessage(message);
    object["source"] = id.toString();
    QJsonDocument document(object);
    sendMulticast(document.toJson(QJsonDocument::JsonFormat::Compact));
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
    QJsonDocument document(object);

    QByteArray data = document.toJson(QJsonDocument::JsonFormat::Compact);
    QMetaObject::invokeMethod(
        &m_network,
        "sendUDP",
        Qt::QueuedConnection,
        Q_ARG(QByteArray, std::move(data))
    );
}

} // namespace remote
