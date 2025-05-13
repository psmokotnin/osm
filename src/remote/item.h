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
#ifndef REMOTE_ITEM_H
#define REMOTE_ITEM_H

#include "abstract/source.h"
#include <QTimer>
#include <QJsonArray>

namespace remote {

class Item : public Abstract::Source
{

private:
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)

public:
    Item(QObject *parent = nullptr);
    void connectProperties();

    enum State {
        WAIT    = 1,
        UPDATED = 2,
        ERROR_STATE = 3
    };
    Q_ENUM(State);

    Shared::Source clone() const override;
    bool cloneable() const override;

    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    QJsonObject metaJsonObject(QString propertyName = {}) const;

    QUuid serverId() const;
    void setServerId(const QUuid &serverId);

    QUuid sourceId() const;
    void setSourceId(const QUuid &dataId);

    bool originalActive() const;
    void setOriginalActive(bool originalActive);

    void applyData(const QJsonArray &data, const QJsonArray &timeData);

    State state() const;
    void setState(const State &state);

    QString host() const;
    void setHost(const QString &host);

    void setEventSilence(bool eventSilence);

public slots:
    Q_INVOKABLE void refresh();
    void dataError(const uint hash, const bool deactivate);
    void dataReceived(const uint hash, const QJsonArray &data, const QJsonArray &timeData);

signals:
    void stateChanged();
    void hostChanged();
    void updateData(remote::Item *);
    void localChanged(QString);
    void sendCommand(QString, QVariant arg = {});

private slots:
    void startResetTimer();
    void resetState();
    void properiesChanged();

private:
    bool m_originalActive;
    bool m_eventSilence;
    QUuid m_serverId, m_sourceId;
    QString m_host;
    State m_state;
    QTimer m_stateTimer;
};

} // namespace remote

#endif // REMOTE_ITEM_H
