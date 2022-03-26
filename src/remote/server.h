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
#ifndef REMOTE_SERVER_H
#define REMOTE_SERVER_H

#include <QObject>
#include "network.h"

class SourceList;
namespace remote {

class Server : public QObject
{
    Q_OBJECT
    const static int TIMER_INTERVAL = 1000;

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    void setSourceList(SourceList *list);

    bool start();
    void stop();

    QByteArray tcpCallback(const QHostAddress &address, const QByteArray &data) const;

signals:

private slots:
    void sendHello();

private:
    QJsonObject prepareMessage(const QString &message) const;
    void sourceNotify(const QUuid &id, const QString &message);
    void sendMulticast(const QByteArray &data);

    QUuid m_uuid;
    QTimer m_timer;
    QThread m_networkThread;
    Network m_network;
    SourceList *m_sourceList;

};

} // namespace remote

#endif // REMOTE_SERVER_H
