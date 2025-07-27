/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import OpenSoundMeter 1.0
import Measurement 1.0
import "qrc:/elements"

Item {
    property var dataObject
    property var dataObjectData : dataObject.data
    readonly property int elementWidth: 200
    property string saveas: "osm"

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true

            DropDown {
                id: modeSelect
                model: dataObjectData.modes
                currentIndex: dataObjectData.mode
                displayText: (dataObjectData.mode === Measurement.LFT ? "LTW" : (modeSelect.width > 120 ? "Power:" : "") + currentText)
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Transfrom mode")
                onCurrentIndexChanged: dataObjectData.mode = currentIndex;
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                model: [44100, 48000, 96000, 192000]
                currentIndex: model.indexOf(dataObjectData.sampleRate)
                onCurrentValueChanged: dataObjectData.sampleRate = currentValue;

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Sample rate")
                Layout.preferredWidth: elementWidth
            }

            SelectableSpinBox {
                value: dataObjectData.size
                onValueChanged: dataObjectData.size = value
                from: 1
                to: 32
                editable: true

                ToolTip.visible: hovered
                ToolTip.text: qsTr("filter count")
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
                Layout.preferredWidth: elementWidth - 30
                Layout.minimumWidth: 100
                Layout.alignment: Qt.AlignVCenter
            }

            RowLayout {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            DropDown {
                model: ["osm"]
                displayText: qsTr("Export to");
                Layout.preferredWidth: elementWidth

                ToolTip.visible: hovered
                ToolTip.text: qsTr("export data")

                onActivated: function() {
                    saveas = currentText;
                    fileDialog.open();
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    FileDialog {
        id: fileDialog
        selectExisting: false
        title: "Please choose a file's name"
        folder: (typeof shortcuts !== 'undefined' ? shortcuts.home : Filesystem.StandardFolder.Home)
        defaultSuffix: saveas
        onAccepted: {
            switch (saveas) {
                case "osm":
                    dataObjectData.save(fileDialog.fileUrl);
                    break;
            }
        }
    }
}
