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

Network::Network(QObject *parent) : QObject(parent), m_udpSocket(nullptr), m_tcpServer(nullptr), m_tcpCallback(nullptr)
{
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &Network::readUDP);
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<remote::Network::responseErrorCallbacks>("remote::Network::responseErrorCallbacks");
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
    if (!m_tcpServer) {
        m_tcpServer = new QTcpServer(this);
        connect(m_tcpServer, &QTcpServer::newConnection, this, &Network::newTCPConnection);
    }
    return m_tcpServer->listen(QHostAddress::AnyIPv4, DEFAULT_PORT);
}

void Network::stopTCPServer()
{
    if (m_tcpServer) {
        m_tcpServer->close();
    }
}

bool Network::bindUDP()
{
    auto connected = m_udpSocket->bind(QHostAddress::AnyIPv4, DEFAULT_PORT);
    if (!connected) {
        qWarning() << "couldn't bind AnyIPv4 at port" << DEFAULT_PORT;
        return false;
    }
    joinMulticast();
    return true;
}

void Network::joinMulticast()
{
    m_udpSocket->joinMulticastGroup(Network::MULTICAST_ADDRESS);
}

void Network::unbindUDP()
{
    if (m_udpSocket) {
        m_udpSocket->close();
    }
}

void Network::readUDP() noexcept
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        emit datagramRecieved(datagram.senderAddress(), datagram.senderPort(), datagram.data());
    }
}

bool Network::sendUDP(const QByteArray &data, const QString &host, quint16 port) const noexcept
{
    QHostAddress destination = (!host.isNull() ? QHostAddress(host) : QHostAddress(QHostAddress::Broadcast));

    qint64 sent = m_udpSocket->writeDatagram(data, destination, port);
    return (sent == data.size());
}

void Network::newTCPConnection()
{
    QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();
    if (!clientConnection) {
        return;
    }

    clientConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    clientConnection->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    auto reciever = new TCPReciever(clientConnection);

    connect(reciever, &TCPReciever::readyRead, this, [ = ]() {
        if (!clientConnection->isWritable()) {
            return;
        }
        if (m_tcpCallback) {
            auto answer = qCompress(m_tcpCallback(std::move(clientConnection->peerAddress()), std::move(reciever->data())));
            auto header = TCPReciever::makeHeader(answer);
            clientConnection->write(header.data(), header.size());

            auto data_ptr = answer.data_ptr()->data();
            int sent = 0, len;
            while (sent < answer.size() && clientConnection->isWritable()) {
                len = std::min(answer.size() - sent, 32767);
                clientConnection->write(data_ptr + sent, len);
                sent += len;
            }
            clientConnection->flush();
        }
        clientConnection->disconnectFromHost();
    });

    connect(reciever, &TCPReciever::timeOut, this, [ = ]() {
        if (clientConnection) {
            clientConnection->close();
        }
    });

    connect(clientConnection, &QTcpSocket::disconnected, this, [ = ]() {
        reciever->deleteLater();
        clientConnection->deleteLater();
    });
}

void Network::sendTCP(const QByteArray &data, const QString host, quint16 port,
                      responseErrorCallbacks callbacks)
{
    auto context = std::get<0>(callbacks);
    auto callback = std::get<1>(callbacks);
    auto onError = std::get<2>(callbacks);

    auto *socketThread = new QThread();
    QTcpSocket *socket = new QTcpSocket();

    socket->moveToThread(socketThread);
    socket->setProxy(QNetworkProxy::NoProxy);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    connect(socketThread, &QThread::started, socketThread, [ = ]() {
        auto reciever =  new TCPReciever(socket);

        connect(reciever, &TCPReciever::readyRead, context.get(), [ = ]() {
            callback(qUncompress(reciever->data()));
            if (socket) {
                socket->disconnectFromHost();
            }
        }, Qt::DirectConnection);

        connect(reciever, &TCPReciever::timeOut, context.get(), [ = ]() {
            qInfo() << "Can't connect to the device" << host << port << ". timeout expired.";
            if (socket) {
                socket->abort();
            }
            onError();
        }, Qt::DirectConnection);

        connect(socket, &QTcpSocket::disconnected, this, [ = ]() {
            if (context) {
                reciever->disconnect(context.get());
            }
            socketThread->exit();
        });

        socket->connectToHost(host, port);

    }, Qt::DirectConnection);

    auto header = TCPReciever::makeHeader(data);
    connect(socket, &QTcpSocket::connected, socketThread, [ = ]() {
        socket->write(header.data(), header.size());
        socket->write(data);
        socket->flush();
    }, Qt::DirectConnection);

    connect(socket, &QTcpSocket::errorOccurred, this, [ = ]([[maybe_unused]] auto socketError) {
        qDebug() << socketError;
        onError();
        socket->close();
    });

    connect(socketThread, &QThread::finished, this, [ = ]() {
        delete socket; //its QThread has finished
        socketThread->deleteLater();
    }, Qt::DirectConnection);

    socketThread->start();
}

} // namespace remote
