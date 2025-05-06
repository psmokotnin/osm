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
#ifndef REMOTE_GROUPITEM_H
#define REMOTE_GROUPITEM_H

#include "remote/item.h"
#include "meta/metagroup.h"

namespace remote {

class GroupItem : public remote::Item, public Meta::Group
{
    Q_OBJECT
    Q_PROPERTY(unsigned size READ size  NOTIFY sizeChanged)
    Q_PROPERTY(SourceList *sourceList READ sourceList CONSTANT)

public:
    explicit GroupItem(QObject *parent = nullptr);

    SourceList *sourceList() override;
    unsigned    size() const override;

    Q_INVOKABLE Shared::Source  pop(const QUuid &) override;

signals:
    void sizeChanged() override;

private:
    SourceList  m_sourceList;
};

} // namespace remote

#endif // REMOTE_GROUPITEM_H
