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
    ~Network();
    static const int DEFAULT_PORT = 49007;
    static const QString MULTICAST_IP;
    static const QHostAddress MULTICAST_ADDRESS;

    typedef const std::function<void(const QByteArray &)> responseCallback;
    typedef const std::function<void()> errorCallback;
    typedef std::function<TCPReciever*(void)> createTCPReciver;
    typedef std::function < QByteArray (const QHostAddress &&, const QByteArray &&) > tcpCallback;
    typedef std::tuple<const std::shared_ptr<QObject>, responseCallback, errorCallback> responseErrorCallbacks;

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
    void joinMulticast();
    void unbindUDP();

    bool sendUDP(const QByteArray &data, const QString &host = QString(), quint16 port = DEFAULT_PORT) const noexcept;
    void sendTCP(const QByteArray &data, const QString host, quint16 port,
                 remote::Network::responseErrorCallbacks callbacks) ;

    void readUDP() noexcept;

signals:
    void datagramRecieved(QHostAddress senderAddress, int senderPort, const QByteArray &);

protected slots:
    void newTCPConnection();

private:
    QUdpSocket *m_udpSocket;
    QTcpServer *m_tcpServer;
    tcpCallback m_tcpCallback;
};

} // namespace remote

#endif // REMOTE_NETWORK_H
