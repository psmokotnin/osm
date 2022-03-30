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

#include "chart/source.h"
#include <QTimer>


namespace remote {

class Item : public chart::Source
{

private:
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)

public:
    Item(QObject *parent = nullptr);
    enum State {
        WAIT    = 1,
        UPDATED = 2,
        ERROR   = 3
    };
    Q_ENUM(State);

    Source *clone() const override;
    bool cloneable() const override;

    Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    QUuid serverId() const;
    void setServerId(const QUuid &serverId);

    QUuid sourceId() const;
    void setSourceId(const QUuid &dataId);

    bool originalActive() const;
    void setOriginalActive(bool originalActive);

    void applyData(const QJsonArray &data);

    State state() const;
    void setState(const State &state);

    QString host() const;
    void setHost(const QString &host);

signals:
    void stateChanged();
    void hostChanged();

private slots:
    void startResetTimer();
    void resetState();

private:
    QUuid m_serverId, m_sourceId;
    QString m_host;
    bool m_originalActive;
    State m_state;
    QTimer m_stateTimer;
};

} // namespace remote

#endif // REMOTE_ITEM_H
