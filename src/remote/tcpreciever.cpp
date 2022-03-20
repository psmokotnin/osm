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

TCPReciever::TCPReciever(QTcpSocket *socket) : QObject(socket), m_packet()
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
    return m_packet;
}

QTcpSocket *TCPReciever::socket() const noexcept
{
    return static_cast<QTcpSocket *>(parent());
}

} // namespace remote
