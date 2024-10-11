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
#include "source_shared.h"
#include "source/source_abstract.h"

namespace Source {

Shared::Shared(std::shared_ptr<Source::Abstract> ptr)
    : std::shared_ptr<Source::Abstract>(ptr)
{
}

Shared::~Shared() = default;

QUuid Shared::uuid() const
{
    if (get()) {
        return get()->uuid();
    }
    return {};
}

QColor Shared::color() const
{
    if (get()) {
        return get()->color();
    }
    return {};
}

QString Shared::objectName() const
{
    if (get()) {
        return get()->objectName();
    }
    return {};
}

} // namespace Source
