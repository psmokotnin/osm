/**
 *  OSM
 *  Copyright (C) 2025  Pavel Smokotnin

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
#ifndef SHARED_SOURCELIST_H
#define SHARED_SOURCELIST_H

#include <memory>
#include <QObject>

class SourceList;

namespace Shared {

class SourceList : public std::shared_ptr<::SourceList>
{
    Q_GADGET
    Q_PROPERTY(::SourceList *data READ get CONSTANT)

public:
    SourceList(std::shared_ptr<::SourceList> ptr = nullptr);
    ~SourceList();

};

} // namespace Shared

Q_DECLARE_METATYPE(Shared::SourceList)

#endif // SHARED_SOURCELIST_H
