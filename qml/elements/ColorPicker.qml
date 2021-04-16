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
import QtQuick 2.7
import QtQuick.Dialogs 1.2

Item {
    id: picker
    property color color

    Rectangle {
        id: preview
        width: picker.width
        height: picker.height
        color: picker.color
    }

    MouseArea {
        property int mouseButtonClicked: Qt.NoButton
        anchors.fill: preview
        cursorShape: "PointingHandCursor"
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            if (pressedButtons & Qt.LeftButton) {
                mouseButtonClicked = Qt.LeftButton
            } else if (pressedButtons & Qt.RightButton) {
                mouseButtonClicked = Qt.RightButton
            }
        }
        onClicked: function(e) {
            if (mouseButtonClicked === Qt.LeftButton) {
                colorDialog.open()
            } else if (mouseButtonClicked === Qt.RightButton) {
                picker.color = sourceList.nextColor();
            }
        }
    }

    ColorDialog {
        id: colorDialog
        title: qsTr("Please choose a color")
        color: picker.color

        onAccepted: {
            picker.color = color
        }
    }
}
