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
import QtQuick 2.12
import QtQuick.Controls 2.15

Dialog {
    id: dialog
    title: ""
    modal: true
    focus: true
    x: (parent ? (parent.width  - width) / 2 : 100)
    y: (parent ? (parent.height - height) / 2 : 100)
    standardButtons: Dialog.Ok | Dialog.Cancel
    onAccepted: {}
    onRejected: {}

    Item {
        focus: true
        anchors.fill: parent
        visible: dialog.visible
        Keys.onReturnPressed: {
            dialog.accept();
        }
    }
}
