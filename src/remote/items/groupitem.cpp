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
#include "groupitem.h"

namespace remote {

GroupItem::GroupItem(QObject *parent)
    : remote::Item{parent}, m_sourceList(this, false)
{
    setObjectName("RemoteGroup");
}

SourceList *GroupItem::sourceList()
{
    return &m_sourceList;
}

unsigned int GroupItem::size() const
{
    return m_sourceList.size();
}

Shared::Source GroupItem::pop(const QUuid &)
{
    return {};
}

} // namespace remote
