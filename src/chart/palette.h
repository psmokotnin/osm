/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#ifndef PALETTE_H
#define PALETTE_H

#include <QObject>
#include <QColor>

namespace chart {

class Palette: public QObject
{
    Q_OBJECT

    QColor m_lineColor, m_centerLineColor, m_textColor, m_backgroundColor;
    bool m_darkMode;
    void initColors() noexcept;

public:
    Palette(QObject *parent);
    bool darkMode()const noexcept
    {
        return m_darkMode;
    }
    void setDarkMode(bool darkMode) noexcept;

    QColor lineColor() const noexcept
    {
        return m_lineColor;
    }
    QColor centerLineColor() const noexcept
    {
        return m_centerLineColor;
    }
    QColor textColor() const noexcept
    {
        return m_textColor;
    }
    QColor backgroundColor() const noexcept
    {
        return m_backgroundColor;
    }

signals:
    void changed();
};

}
#endif // PALETTE_H
