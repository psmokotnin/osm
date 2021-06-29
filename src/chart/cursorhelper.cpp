/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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

#include "cursorhelper.h"

namespace chart {

CursorHelper::CursorHelper(QObject *parent) : QObject(parent)
{
}

qreal CursorHelper::value(QString key) const noexcept
{
    if (m_values.contains(key)) {
        return m_values[key];
    }
    return NAN;
}

void CursorHelper::setValue(QString key, qreal value) noexcept
{
    m_values[key] = value;
    emit valueUpdated();
}

void CursorHelper::unsetValue(QString key) noexcept
{
    m_values[key] = NAN;
}

} // namespace chart
