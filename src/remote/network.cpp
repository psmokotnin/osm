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
#include "network.h"
#include <qsysinfo.h>
#include <QVector>
#include "tcpreciever.h"

namespace remote {

const QString Network::MULTICAST_IP = "239.255.42.42";
const QHostAddress Network::MULTICAST_ADDRESS = QHostAddress{Network::MULTICAST_IP};

Network::Network(QObject *parent) : QObject(parent), tcpServer(this), m_tcpCallback(nullptr)
{
    udpSocket = new QUdpSocket(this);
    udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    connect(&tcpServer, &QTcpServer::newConnection, this, &Network::newTCPConnection);
    qRegisterMetaType<QHostAddress>("QHostAddress");
}

Network::~Network()
{
    unbindUDP();
    stopTCPServer();
}

void Network::setTcpCallback(Network::tcpCallback callback) noexcept
{
    m_tcpCallback = callback;
}

bool Network::startTCPServer()
{
    stopTCPServer();
    return tcpServer.listen(QHostAddress::AnyIPv4, DEFAULT_PORT);
}

void Network::stopTCPServer()
{
    tcpServer.close();
}

bool Network::bindUDP()
{
    const QHostAddress &address = QHostAddress::AnyIPv4;
    auto connected = udpSocket->bind(address, DEFAULT_PORT);
    if (!connected) {
        qWarning() << "couldn't bind IPv4:" << address.toIPv4Address()  << " port" << DEFAULT_PORT;
        return false;
    }
    connect(udpSocket, &QUdpSocket::readyRead, this, &Network::readUDP);
    return true;
}

void Network::joinMulticast()
{
    udpSocket->joinMulticastGroup(Network::MULTICAST_ADDRESS);
}

void Network::unbindUDP()
{
    if (udpSocket->isOpen())
        udpSocket->close();
}

void Network::readUDP() const noexcept
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        emit datagramRecieved(datagram.senderAddress(), datagram.senderPort(), datagram.data());
    }
}

bool Network::sendUDP(const QByteArray &data, const QString &host, quint16 port) const noexcept
{
    QHostAddress destination = (!host.isNull() ? QHostAddress(host) : QHostAddress(QHostAddress::Broadcast));

    qint64 sent = udpSocket->writeDatagram(data, destination, port);
    return (sent == data.size());
}

void Network::newTCPConnection()
{
    QTcpSocket *clientConnection = tcpServer.nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);

    auto reciever = new TCPReciever();

    connect(reciever, &TCPReciever::readyRead, [ = ]() {
        if (m_tcpCallback) {
            auto answer = m_tcpCallback(clientConnection->peerAddress(), reciever->data());
            auto header = TCPReciever::makeHeader(answer);
            clientConnection->write(header.data(), header.size());
            clientConnection->waitForBytesWritten();

            auto data_ptr = answer.data_ptr()->data();
            int sent = 0, len;
            while (sent < answer.size() && clientConnection->isWritable()) {
                len = std::min(answer.size() - sent, 32767);
                clientConnection->write(data_ptr + sent, len);
                sent += len;
                clientConnection->waitForBytesWritten();
            }
        }
        clientConnection->close();
        clientConnection->deleteLater();
    });

    connect(reciever, &TCPReciever::timeOut, [ = ]() {
        if (clientConnection) {
            clientConnection->close();
            clientConnection->deleteLater();
        }
    });
    reciever->setSocket(clientConnection);
}

void Network::sendTCP(const QByteArray &data, const QString &host, quint16 port, Network::responseCallback callback,
                      Network::errorCallback onError) noexcept
{
    QHostAddress destination = (host.isNull() ? QHostAddress(host) : QHostAddress(QHostAddress::Broadcast));
    auto *socketThread = new QThread();
    TCPReciever *reciever = nullptr;
    QTcpSocket *socket = new QTcpSocket();
    socket->moveToThread(socketThread);
    if (callback) {
        reciever =  new TCPReciever();
        reciever->moveToThread(socketThread);
        reciever->setSocket(socket);
    }

    connect(socketThread, &QThread::started, [ = ]() {
        socket->connectToHost(host, port);
    });

    auto header = TCPReciever::makeHeader(data);
    connect(socket, &QTcpSocket::connected, [ = ]() {
        socket->write(header.data(), header.size());
        socket->waitForBytesWritten();
        socket->write(data);
        socket->waitForBytesWritten();
    });

    if (reciever) {
        connect(reciever, &TCPReciever::readyRead, [ = ]() {
            callback(reciever->data());
            socket->disconnectFromHost();
        });

        connect(reciever, &TCPReciever::timeOut, [ = ]() {
            qInfo() << "Can't connect to the device. timeout expired.";
            socket->disconnectFromHost();
            onError();
        });
    }

    connect(socket, &QTcpSocket::disconnected, [ = ]() {
        socket->close();
        socketThread->exit();
    });

    connect(socket, &QTcpSocket::errorOccurred, [ = ]([[maybe_unused]] auto socketError) {
        //qDebug() << "socketError" << socketError;
        onError();
        socket->close();
        socketThread->exit();
    });

    connect(socketThread, &QThread::finished, [ = ]() {
        socket->deleteLater();
        socketThread->deleteLater();
    });

    socketThread->start();
}

} // namespace remote
