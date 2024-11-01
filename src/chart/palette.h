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

namespace Chart {

class Palette: public QObject
{
    Q_OBJECT

public:
    Palette(QObject *parent);
    const bool &darkMode()const noexcept;
    void setDarkMode(bool darkMode) noexcept;

    const QColor &lineColor() const noexcept;
    const QColor &centerLineColor() const noexcept;
    const QColor &textColor() const noexcept;
    const QColor &backgroundColor() const noexcept;
    const QColor &cursorLineColor() const noexcept;

    const float &lineWidth(const bool &highlighted) const noexcept;

signals:
    void changed();

private:
    void initColors() noexcept;

    QColor m_lineColor, m_centerLineColor, m_textColor, m_backgroundColor, m_cursorLineColor;
    bool m_darkMode;
    float m_lineWidth, m_highlightedLineWidth;
};

}
#endif // PALETTE_H
