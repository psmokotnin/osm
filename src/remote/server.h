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
#include "shared/source_shared.h"
#include "source/group.h"

class SourceList;
class Generator;
namespace Source {
class Abstract;
}

namespace remote {

class Server : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString lastConnected READ lastConnected NOTIFY lastConnectedChanged)
    Q_PROPERTY(bool generatorEnable READ generatorEnable WRITE setGeneratorEnable NOTIFY generatorEnableChanged)
    const static int TIMER_INTERVAL = 1000;

public:
    explicit Server(std::shared_ptr<Generator> generator, QObject *parent = nullptr);
    ~Server();

    void setSourceList(SourceList *list);

    bool start();
    void stop();

    bool active() const;
    void setActive(bool state);

    QByteArray tcpCallback(const QHostAddress &&address, const QByteArray &&data);
    QString lastConnected() const;

    bool generatorEnable() const;
    void setGeneratorEnable(bool newGeneratorEnable);

signals:
    void activeChanged();
    void lastConnectedChanged();
    void generatorEnableChanged();

public slots:
    void sendSouceNotify();
    void sendGeneratorNotify();

private slots:
    void sendHello();

private:
    QJsonObject prepareMessage(const QString &message) const;
    void sourceNotify(const Shared::Source &source, const QString &message, const QJsonValue &data = {});
    void sendMulticast(const QByteArray &data);
    void setLastConnected(const QString &lastConnected);
    void connectSourceList(SourceList *list, const Shared::Source &group = {});

    QUuid m_uuid;
    QTimer m_timer;
    QString m_lastConnected;
    QThread m_networkThread;
    Network m_network;
    SourceList *m_sourceList;//TODO PTR
    std::shared_ptr<Generator> m_generator;
    bool m_generatorEnable;
};

} // namespace remote

#endif // REMOTE_SERVER_H
