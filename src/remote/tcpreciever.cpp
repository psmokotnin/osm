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
#include <QtEndian>

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
    QTimer::singleShot(1000, this, [ = ]() {
        emit timeOut();
    });
}

const QByteArray &TCPReciever::data() const noexcept
{
    return m_data;
}

std::array<char, 4> TCPReciever::makeHeader(const QByteArray &data)
{
    qint32 size = qToLittleEndian<qint32>(data.size());
    std::array<char, 4> a;
    std::memmove(a.data(), &size, 4);
    return a;
}

void TCPReciever::socketReadyRead()
{
    if (!p_size.value) {
        const auto sizeData = socket()->read(4);
        p_size.byte[0] = sizeData[0];
        p_size.byte[1] = sizeData[1];
        p_size.byte[2] = sizeData[2];
        p_size.byte[3] = sizeData[3];
        p_size.value = qFromLittleEndian(p_size.value);
    }

    const auto data = socket()->readAll();
    m_data.push_back(data);

    if (p_size.value <= m_data.size()) {
        emit readyRead();
    }
}

QTcpSocket *TCPReciever::socket() const noexcept
{
    return static_cast<QTcpSocket *>(parent());
}

} // namespace remote
