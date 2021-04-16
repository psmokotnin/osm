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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

Item {
    id: i
    implicitWidth: label.implicitWidth + cb.implicitWidth + 20
    implicitHeight: cb.implicitHeight

    property string title : ""
    property var model : []
    property int currentIndex : 0
    property string textRole: ""
    property string valueRole: ""
    property string tooltip : ""

    RowLayout {

        spacing: 1
        anchors.fill: parent

        Label {
            id: label
            Layout.margins: 5
            text: title
        }

        ComboBox {
            id: cb
            Layout.fillWidth: true
            Layout.leftMargin: 5
            Layout.bottomMargin: 5
            model: i.model
            currentIndex: i.currentIndex
            textRole: i.textRole
            valueRole: i.valueRole
            onCurrentIndexChanged: {
                i.currentIndex = currentIndex
            }
            ToolTip.visible: (tooltip ? hovered : none)
            ToolTip.text: tooltip
        }
    }
}
