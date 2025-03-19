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

#include <QJsonArray>
#include "group.h"

namespace Source {

Group::Group(QObject *parent)
    : Abstract{parent},
      m_sourceList(this, false)
{
    qRegisterMetaType<Source::Group *>("Source::Group*");
    setObjectName("Group");
    setName("Group");
    setActive(true);

    connect(&m_sourceList, &SourceList::preItemAppended, this, &Group::sizeChanged);
    connect(&m_sourceList, &SourceList::preItemRemoved, this, &Group::sizeChanged);
}

void Group::destroy()
{
    m_sourceList.clean();
    Abstract::destroy();
}

QJsonObject Group::toJSON(const SourceList *list) const noexcept
{
    auto object = Source::Abstract::toJSON(list);

    object["list"]      = m_sourceList.toJSON(list);

    return object;
}

void Group::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    Source::Abstract::fromJSON(data, list);

    m_sourceList.fromJSON(data["list"].toArray(), list);
}

Shared Group::clone() const
{
    auto cloned = std::make_shared<Group>(parent());
    cloned->setActive(active());
    cloned->setName(name());
    for (const auto& itemPtr : m_sourceList) {
        if (itemPtr == nullptr) {
            continue;
        }
        cloned->m_sourceList.appendItem(itemPtr->clone(), true);
    }
    return std::static_pointer_cast<Source::Abstract>(cloned);
}

void Group::add(const Shared &source)
{
    m_sourceList.appendItem(source);
}

Shared Group::pop(const QUuid &uuid)
{
    auto source = m_sourceList.getByUUid(uuid);
    if (source) {
        remove(uuid, false);
    }
    return source;
}

void Group::remove(const QUuid &uuid, bool deleteItem)
{
    m_sourceList.removeItem(uuid, deleteItem);
}

unsigned int Group::size() const
{
    return m_sourceList.size();
}

SourceList *Group::sourceList()
{
    return &m_sourceList;
}

} // namespace Source
