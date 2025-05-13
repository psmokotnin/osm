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

#include "meta/metagroup.h"
#include "abstract/source.h"
#include "sourcelist.h"

namespace Source {

class Group : public Abstract::Source, public Meta::Group
{
    Q_OBJECT
    Q_PROPERTY(unsigned size READ size  NOTIFY sizeChanged)
    Q_PROPERTY(SourceList *sourceList READ sourceList CONSTANT)

public:
    explicit                    Group(QObject *parent = nullptr);

    Q_INVOKABLE void            destroy() override;
    Shared::Source              clone() const override;

    void                        add(const Shared::Source &source);
    Q_INVOKABLE Shared::Source  pop(const QUuid &uuid) override;
    void                        remove(const QUuid &uuid, bool deleteItem = true);
    unsigned                    size() const override;
    SourceList                 *sourceList() override;


    QJsonObject toJSON() const noexcept override;
    void        fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

signals:
    void sizeChanged() override;

private:
    SourceList  m_sourceList;
};

} // namespace Source

#endif // SOURCE_GROUP_H
