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
#ifndef REMOTE_TCPRECIEVER_H
#define REMOTE_TCPRECIEVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <array>

namespace remote {

class TCPReciever : public QObject
{
    Q_OBJECT
    static const int TIMEOUT = 10000;

public:
    //! To run reciever in the socket's thread use setSocket after moving socket to a new thread
    explicit TCPReciever(QTcpSocket *socket = nullptr);

    void setSocket(QTcpSocket *socket = nullptr);
    const QByteArray &data() const noexcept;

    static std::array<char, 4> makeHeader(const QByteArray &data);

public slots:
    virtual void socketReadyRead();

signals:
    void readyRead();
    void timeOut();

private:
    union {
        qint32 value;
        char byte[4];
    } p_size;
    QByteArray m_data;
    QTcpSocket *socket() const noexcept;
    QTimer m_timer;
};

} // namespace remote

#endif // REMOTE_TCPRECIEVER_H
