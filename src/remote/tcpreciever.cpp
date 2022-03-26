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
#include "tcpreciever.h"
#include <QTimer>

namespace remote {

TCPReciever::TCPReciever(QTcpSocket *socket) : QObject(socket), p_size{0}, m_data()
{
    if (socket)
        setSocket(socket);
}

void TCPReciever::setSocket(QTcpSocket *socket)
{
    setParent(socket);
    connect(socket, &QTcpSocket::readyRead, this, &TCPReciever::socketReadyRead);

    //run timeout timer
    QTimer::singleShot(10000, this, [ = ]() {
        emit timeOut();
    });
}

const QByteArray &TCPReciever::data() const noexcept
{
    return m_data;
}

QByteArray TCPReciever::prepareForSend(const QByteArray &data)
{
    //TODO: ENDIAN!
    qint32 size = static_cast<qint32>(data.size());
    return QByteArray::fromRawData(reinterpret_cast<const char *>(&size), 4) + data;
}

void TCPReciever::socketReadyRead()
{
    const auto data = socket()->readAll();
    m_data.push_back(data);

    if (!p_size.value) {
        auto packetSizeQBA = m_data.left(4);
        p_size.byte[0] = packetSizeQBA[0];
        p_size.byte[1] = packetSizeQBA[1];
        p_size.byte[2] = packetSizeQBA[2];
        p_size.byte[3] = packetSizeQBA[3];
        m_data.remove(0, 4);
    }
    if (p_size.value <= m_data.size()) {
        emit readyRead();
    }
}

QTcpSocket *TCPReciever::socket() const noexcept
{
    return static_cast<QTcpSocket *>(parent());
}

} // namespace remote
