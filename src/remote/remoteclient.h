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
#include "apikey.h"
#include "settings.h"

class SourceList;
namespace remote {

class Item;
class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool licensed READ licensed NOTIFY licenseChanged)
    Q_PROPERTY(QString licenseOwner READ licenseOwner NOTIFY licenseChanged)
    Q_PROPERTY(bool staticLicense READ staticLicense CONSTANT)
    const static int TIMER_INTERVAL = 250;
    const static QString SETTINGS_LICENSE_KEY;

public:
    explicit Client(Settings *settings, QObject *parent = nullptr);
    ~Client();
    void setSourceList(SourceList *list);

    bool start();
    void stop();

    bool active() const;
    void setActive(bool state);

    Q_INVOKABLE bool openLicenseFile(const QUrl &fileName);
    bool licensed() const;
    QString licenseOwner() const;

    bool staticLicense() const;

public slots:
    void dataRecieved(QHostAddress senderAddress, int senderPort, const QByteArray &data);

signals:
    void activeChanged();
    void licenseChanged();

private slots:
    void sendRequests();
    void requestUpdate(remote::Item *item);
    void sendCommand(remote::Item *item, QString command);

private:
    remote::Item *addItem(const QUuid &serverId, const QUuid &sourceId, const QString &objectName, const QString &host);
    void sendUpdate(Item *item, QString propertyName);
    void requestChanged(Item *item);
    void requestData(Item *item);
    void requestSource(Item *item, const QString &message, Network::responseCallback callback,
                       Network::errorCallback errorCallback = 0, QJsonObject itemData = {});

    Network m_network;
    ApiKey m_key;
    Settings *m_settings;
    QThread m_thread;
    QTimer m_timer;
    SourceList *m_sourceList;
    QMap<unsigned int, std::pair<QHostAddress, int>> m_servers;
    QMap<unsigned int, Item *> m_items;

    bool m_onRequest;
    typedef unsigned long UpdateKey;
    const UpdateKey READY_FOR_UPDATE = std::numeric_limits<UpdateKey>::max();
    const UpdateKey ON_UPDATE = READY_FOR_UPDATE - 1;
    std::atomic<UpdateKey> m_updateCounter;
    QMap<unsigned int, UpdateKey> m_needUpdate;
};

} // namespace remote

#endif // REMOTE_CLIENT_H
