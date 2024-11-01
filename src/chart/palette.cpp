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
#include "palette.h"
using namespace Chart;

Palette::Palette(QObject *parent):
    QObject(parent),
    m_lineColor(), m_textColor(), m_backgroundColor(),
    m_darkMode(false), m_lineWidth(1.5), m_highlightedLineWidth(3)
{
    initColors();
}
void Palette::initColors() noexcept
{
    if (m_darkMode) {
        m_lineColor         = QColor(255, 255, 255, 40);
        m_cursorLineColor   = QColor("#607D8B");
        m_centerLineColor   = QColor(255, 255, 255, 128);
        m_textColor         = QColor(255, 255, 255, 255);
        m_backgroundColor   = QColor(Qt::black);
    } else {
        m_lineColor         = QColor(0, 0, 0, 25);
        m_cursorLineColor   = QColor("#B0BEC5");
        m_centerLineColor   = QColor(0, 0, 0, 128);
        m_textColor         = QColor(0, 0, 0, 255);
        m_backgroundColor   = QColor(Qt::white);
    }
    m_cursorLineColor.setAlpha(128);
}
const bool &Palette::darkMode() const noexcept
{
    return m_darkMode;
}
void Palette::setDarkMode(bool darkMode) noexcept
{
    if (darkMode != m_darkMode) {
        m_darkMode = darkMode;
        initColors();
        emit changed();
    }
}

const QColor &Palette::lineColor() const noexcept
{
    return m_lineColor;
}

const QColor &Palette::centerLineColor() const noexcept
{
    return m_centerLineColor;
}

const QColor &Palette::textColor() const noexcept
{
    return m_textColor;
}

const QColor &Palette::backgroundColor() const noexcept
{
    return m_backgroundColor;
}

const QColor &Palette::cursorLineColor() const noexcept
{
    return m_cursorLineColor;
}

const float &Palette::lineWidth(const bool &highlighted) const noexcept
{
    return highlighted ? m_highlightedLineWidth : m_lineWidth;
}
