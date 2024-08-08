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

import StandardLine 1.0
import SourceModel 1.0
import "qrc:/elements"

Item {
    property var dataObject
    property var dataObjectData : dataObject.data

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            DropDown {
                id: modeSelect
                model: dataObjectData.modes
                currentIndex: dataObjectData.mode
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Standard")
                onCurrentIndexChanged: dataObjectData.mode = currentIndex;
                Layout.preferredWidth: 200
            }

            SelectableSpinBox {
                value: dataObjectData.loudness
                onValueChanged: dataObjectData.loudness = value
                from: 20
                to: 100
                editable: true
                visible: dataObjectData.mode === StandardLine.ELC
            }

            Label {
                text: qsTr("phon")
                visible: dataObjectData.mode === StandardLine.ELC

                horizontalAlignment: Text.AlignLeft
            }

            Item {
                Layout.fillWidth: true
            }

            ColorPicker {
                id: colorPicker
                Layout.preferredWidth: 25
                Layout.preferredHeight: 25
                Layout.margins: 0

                onColorChanged: {
                    dataObjectData.color = color
                }

                Component.onCompleted: {
                    color = dataObjectData.color
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("series color")
            }

            NameField {
                id:titleField
                target: dataObject
                implicitWidth: 100
                Layout.alignment: Qt.AlignVCenter
            }
        }

        RowLayout {}
    }
}
