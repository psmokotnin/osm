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
#ifndef REMOTE_CLIENT_H
#define REMOTE_CLIENT_H

#include <QObject>
#include <QList>
#include "network.h"

class SourceList;
namespace remote {

class Item;
class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);
    void setSourceList(SourceList *list);

public slots:
    void dataRecieved(QHostAddress senderAddress, int senderPort, const QByteArray &data);

signals:

private:
    void requestChanged(Item *item);
    void requestData(Item *item);
    void requestSource(Item *item, const QString &message, Network::responseCallback callback);

    Network m_network;
    SourceList *m_sourceList;
    QMap<unsigned int, std::pair<QHostAddress, int>> m_servers;
    QMap<unsigned int, Item *> m_items;
};

} // namespace remote

#endif // REMOTE_CLIENT_H
