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

#ifndef CHART_CURSORHELPER_H
#define CHART_CURSORHELPER_H

#include <QObject>
#include <QMap>

namespace Chart {

class CursorHelper : public QObject
{
    Q_OBJECT

public:
    explicit CursorHelper(QObject *parent = nullptr);

    Q_INVOKABLE qreal value(QString key, QString axis) const noexcept;
    Q_INVOKABLE void setValue(QString key, QString axis, qreal value) noexcept;
    Q_INVOKABLE void unsetValue(QString key, QString axis) noexcept;

signals:
    void valueUpdated();

protected:
    QMap<QString, qreal> m_values = {};
};

} // namespace chart

#endif // CHART_CURSORHELPER_H
