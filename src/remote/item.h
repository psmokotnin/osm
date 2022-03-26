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

namespace remote {

class Item : public chart::Source
{
    Q_OBJECT

public:
    Item(QObject *parent = nullptr);

    Source *clone() const override;

    Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    QUuid serverId() const;
    void setServerId(const QUuid &serverId);

    QUuid sourceId() const;
    void setSourceId(const QUuid &dataId);

    bool originalActive() const;
    void setOriginalActive(bool originalActive);

    void applyData(const QJsonArray &data);

private:
    QUuid m_serverId, m_sourceId;
    bool m_originalActive;
};

} // namespace remote

#endif // REMOTE_ITEM_H
