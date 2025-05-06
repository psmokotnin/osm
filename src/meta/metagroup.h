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

#ifndef META_GROUP_H
#define META_GROUP_H

#include <QObject>
#include "meta/metabase.h"
#include "sourcelist.h"
#include "shared/source_shared.h"

namespace Meta {

class Group : public Meta::Base
{
public:
    Group() = default;

    virtual SourceList *sourceList() = 0;
    virtual unsigned    size() const = 0;

    virtual Shared::Source  pop(const QUuid &) = 0;

//virtual signals:
    virtual void sizeChanged() = 0;
};

} // namespace meta

#endif // META_GROUP_H
