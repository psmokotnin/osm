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
#include <QtMath>

namespace Chart {

CursorHelper::CursorHelper(QObject *parent) : QObject(parent)
{
}

qreal CursorHelper::value(QString key, QString axis) const noexcept
{
    if (m_values.contains(key.isEmpty() ? axis : key)) {
        return m_values[key.isEmpty() ? axis : key];
    }
    return NAN;
}

void CursorHelper::setValue(QString key, QString axis, qreal value) noexcept
{
    m_values[key.isEmpty() ? axis : key] = value;
    emit valueUpdated();
}

void CursorHelper::unsetValue(QString key, QString axis) noexcept
{
    m_values[key.isEmpty() ? axis : key] = NAN;
    emit valueUpdated();
}

} // namespace chart
