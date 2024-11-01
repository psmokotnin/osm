/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#include <cmath>
#include "painteditem.h"

using namespace Chart;

PaintedItem::PaintedItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{

}

float PaintedItem::pwidth() const noexcept
{
    return widthf()  - (padding.left + padding.right);
}

float PaintedItem::pheight() const noexcept
{
    return heightf() - (padding.top  + padding.bottom);
}

float PaintedItem::widthf() const noexcept
{
    return static_cast<float>(width());
}

float PaintedItem::heightf() const noexcept
{
    return static_cast<float>(height());
}

QString PaintedItem::format(float v)
{
    bool addK = false;
    if (v >= 1000.f) {
        v /= 1000.f;
        addK = true;
    }
    v = std::round(v * 10.f) / 10.f;
    return QString::number(static_cast<double>(v)) + (addK ? "K" : "");
}
