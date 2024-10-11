/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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
#ifndef SOURCE_GROUP_H
#define SOURCE_GROUP_H

#include <QObject>
#include <QColor>
#include <QUuid>

#include "source/source_abstract.h"
#include "sourcelist.h"

namespace Source {

class Group : public Abstract
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ expanded WRITE setExpanded NOTIFY expandedChanged)
    Q_PROPERTY(bool sync READ sync WRITE setSync NOTIFY syncChanged)
    Q_PROPERTY(unsigned size READ size  NOTIFY sizeChanged)
    Q_PROPERTY(SourceList *sourceList READ sourceList NOTIFY sourceListChanged)

public:
    explicit Group(QObject *parent = nullptr);
    Q_INVOKABLE void destroy() override;

    bool expanded() const;
    void setExpanded(bool expanded);

    bool sync() const;
    void setSync(bool newSync);

    Source::Shared clone() const override;

    void                        add(const Source::Shared &source);
    Q_INVOKABLE Source::Shared  pop(const QUuid &uuid);
    Q_INVOKABLE void            remove(const QUuid &uuid, bool deleteItem = true);
    unsigned        size() const;

    SourceList *sourceList();

    QJsonObject toJSON(const SourceList *list = nullptr) const noexcept override;
    void        fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

signals:
    void expandedChanged();
    void syncChanged();
    void sizeChanged();

    void sourceListChanged();

private:
    bool m_expanded;
    bool m_sync;

    SourceList  m_sourceList;
};

} // namespace Source

#endif // SOURCE_GROUP_H
