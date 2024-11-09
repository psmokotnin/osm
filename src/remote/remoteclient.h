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
#include "settings.h"
#include "remote/generatorremote.h"

class SourceList;
class Generator;
namespace remote {

class Item;
class GeneratorRemote;

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QStringList generatorsList READ generatorsList NOTIFY generatorsListChanged)
    Q_PROPERTY(SharedGeneratorRemote controlledGenerator READ controlledGenerator NOTIFY controlledGeneratorChanged)

    const static int TIMER_INTERVAL = 250;

public:
    explicit Client(Settings *settings, QObject *parent = nullptr);
    ~Client();
    void setSourceList(SourceList *list);

    bool start();
    void stop();

    bool active() const;
    void setActive(bool state);

    Q_INVOKABLE void reset();

    QStringList generatorsList() const;
    Q_INVOKABLE void selectGenerator(const QString &name);

    SharedGeneratorRemote controlledGenerator() const;
    void setControlledGenerator(const SharedGeneratorRemote &newControlledGenerator);

public slots:
    void processData(QHostAddress senderAddress, int senderPort, const QByteArray &data);

signals:
    void activeChanged();
    void dataError(const uint hash, const bool deactivate);
    void dataReceived(const uint hash, const QJsonArray &data, const QJsonArray &timeData);
    void newRemoteItem(const QUuid &serverId, const QUuid &sourceId, const QString &objectName,
                       const QString &host, const QUuid groupId);
    void generatorsListChanged();
    void controlledGeneratorChanged();

private slots:
    void sendRequests();
    void requestUpdate(const std::shared_ptr<Item> &item);
    void sendCommand(const std::shared_ptr<Item> &item, QString command, QVariant arg);
    void appendItem(const QUuid &serverId, const QUuid &sourceId, const QString &objectName,
                    const QString &host, const QUuid groupId = {});

private:
    std::shared_ptr<Item> addItem(const QUuid &serverId, const QUuid &sourceId, const QString &objectName,
                                  const QString &host, const QUuid groupId = {});
    void requestChanged(const std::shared_ptr<Item> &item);
    void requestGenearatorChanged(const SharedGeneratorRemote &genearator);
    void requestData(const std::shared_ptr<Item> &item);

    template <typename ItemType>
    void sendUpdate(const std::shared_ptr<ItemType> &item, QString propertyName);
    template <typename ItemType>
    void requestSource(const std::shared_ptr<ItemType> &item, const QString &message, Network::responseCallback callback,
                       Network::errorCallback errorCallback = 0, QJsonObject itemData = {});

    Network m_network;
    Settings *m_settings;
    QThread m_thread;
    QTimer m_timer;
    SourceList *m_sourceList;
    QMap<unsigned int, std::pair<QHostAddress, int>> m_servers;
    QMap<unsigned int, std::shared_ptr<Item>> m_items;

    QMap<unsigned int, SharedGeneratorRemote> m_generators;
    SharedGeneratorRemote             m_controlledGenerator;

    std::atomic<bool> m_onRequest;
    typedef unsigned long UpdateKey;
    const UpdateKey READY_FOR_UPDATE = std::numeric_limits<UpdateKey>::max();
    const UpdateKey ON_UPDATE = READY_FOR_UPDATE - 1;
    std::atomic<UpdateKey> m_updateCounter;
    QMap<unsigned int, UpdateKey> m_needUpdate;
};

} // namespace remote

#endif // REMOTE_CLIENT_H
