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
#ifndef PAINTEDITEM_H
#define PAINTEDITEM_H

#include <QtQuick/QQuickPaintedItem>

namespace Chart {
class PaintedItem : public QQuickPaintedItem
{
    Q_OBJECT

public:
    PaintedItem (QQuickItem *parent = Q_NULLPTR);
    float pwidth() const noexcept;
    float pheight() const noexcept;
    float widthf() const noexcept;
    float heightf() const noexcept;

protected:
    QString format(float v);

    const struct Padding {
        float   left    = 50.f,
                right   = 10.f,
                top     = 10.f,
                bottom  = 20.f;
    } padding;
};
}
#endif // PAINTEDITEM_H
