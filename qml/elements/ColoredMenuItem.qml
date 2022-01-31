/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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

import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

MenuItem {
    id: control
    property color checkedColor : Material.primaryColor;

    Rectangle {
        id: coloredRect
        x: text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        z: 2

        width: 18
        height: 18
        opacity: control.checked ? 1 : 0
        color: control.checkedColor

        Image {
            x: 2
            y: 2
            width: 14
            height: 14
            source: "qrc:/qt-project.org/imports/QtQuick/Controls.2/Material/images/check.png"
            fillMode: Image.PreserveAspectFit

            scale: control.checked ? 1 : 0
            Behavior on scale { NumberAnimation { duration: 100 } }
        }
    }
}
