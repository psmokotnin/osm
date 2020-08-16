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
import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.13

import SourceModel 1.0

Item {
    property var dataObject

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            SpinBox {
                value: dataObject.loudness
                onValueChanged: dataObject.loudness = value
                from: 20
                to: 80
            }

            Label {
                text: qsTr("phon")
                Layout.fillWidth: true

                horizontalAlignment: Text.AlignLeft
            }

            ColorPicker {
                id: colorPicker
                Layout.preferredWidth: 25
                Layout.preferredHeight: 25
                Layout.margins: 0

                onColorChanged: {
                    dataObject.color = color
                }

                Component.onCompleted: {
                    color = dataObject.color
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("series color")
            }

            TextField {
                id:titleField
                placeholderText: qsTr("title")
                text: dataObject.name
                onTextEdited: dataObject.name = text
                implicitWidth: 100
                ToolTip.visible: hovered
                ToolTip.text: qsTr("title")
                Layout.alignment: Qt.AlignVCenter
            }
        }

        RowLayout {}
    }
}
