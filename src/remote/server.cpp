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

namespace remote {

Server::Server(QObject *parent) : QObject(parent), m_networkThread(), m_network()
{
    m_network.moveToThread(&m_networkThread);
    m_networkThread.setObjectName("Network");

    connect(&m_networkThread, &QThread::started,  &m_network, &Network::startTCPServer);
    connect(&m_networkThread, &QThread::finished, &m_network, &Network::stopTCPServer);

    m_timer.setInterval(TIMER_INTERVAL);
    connect(&m_timer, &QTimer::timeout, this, &Server::sendHello);
    m_timer.start();
}

Server::~Server()
{
    stop();
}

bool Server::start()
{
    m_networkThread.start();
    return m_networkThread.isRunning();
}

void Server::stop()
{
    m_network.disconnect();
    m_networkThread.quit();
    m_networkThread.wait();
}

void Server::sendHello()
{
    QJsonObject object;
    object["api"] = "Open Sound Meter";
    object["version"] = APP_GIT_VERSION;
    object["message"] = "hello";
    QJsonDocument document(object);

    QByteArray data = document.toJson(QJsonDocument::JsonFormat::Compact);
    QMetaObject::invokeMethod(
        &m_network,
        "sendUDP",
        Qt::QueuedConnection,
        Q_ARG(QByteArray, std::move(data)));
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

} // namespace remote
