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
#ifndef REMOTE_NETWORK_H
#define REMOTE_NETWORK_H

#include <QObject>
#include <QtNetwork>

#include <QDebug>

namespace remote {

class TCPReciever;

class Network : public QObject
{
    Q_OBJECT

public:
    explicit Network(QObject *parent = nullptr);
    static const int DEFAULT_PORT = 49007;
    const QString MULTICAST_IP{"239.255.42.42"};

    typedef const std::function<void(QByteArray)> responseCallback;
    typedef const std::function<void()> errorCallback;
    typedef std::function<TCPReciever*(void)> createTCPReciver;
    typedef std::function<QByteArray (QHostAddress, const QByteArray &)> tcpCallback;

    constexpr quint16 port() const noexcept
    {
        return DEFAULT_PORT;
    };
    void setTcpCallback(tcpCallback callback) noexcept;
    void setTcpReciever(createTCPReciver reciverCreator) noexcept;

public slots:
    bool startTCPServer();
    void stopTCPServer();

    bool bindUDP();
    void unbindUDP();

    bool sendUDP(const QByteArray &data, const QString &host = QString(), quint16 port = DEFAULT_PORT) const noexcept;

    void sendTCP(const QByteArray &data, const QString &host = QString(), quint16 port = DEFAULT_PORT,
                 Network::responseCallback callback = nullptr, Network::errorCallback onError = nullptr) noexcept;

    void readUDP() const noexcept;

signals:
    void datagramRecieved(QHostAddress senderAddress, int senderPort, const QByteArray &) const;

protected slots:
    void newTCPConnection();

private:
    QUdpSocket *udpSocket;
    QTcpServer tcpServer;
    tcpCallback m_tcpCallback;
    createTCPReciver m_reciverCreator;
};

} // namespace remote

#endif // REMOTE_NETWORK_H
