/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.13
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.12

import OpenSoundMeter 1.0
import SourceModel 1.0
import "qrc:/elements"

Item {
    property var dataObject
    property var dataObjectData : dataObject.data
    readonly property int elementWidth: width / 6

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Row 1: source selector, smoothing, colour, name ──────────────
        RowLayout {

            DropDown {
                id: sourceSelect
                model: SourceModel {
                    id: sourceModel
                    addNone: true
                    noneTitle: "None"
                    filter: dataObjectData.uuid
                    unrollGroups: true
                    list: sourceList
                }
                currentIndex: sourceModel.indexOf(dataObjectData.sourceId)
                textRole: "title"
                valueRole: "source"
                Layout.preferredWidth: elementWidth * 2
                ToolTip.visible: hovered
                ToolTip.text: qsTr("source measurement to export")
                onCurrentIndexChanged: {
                    dataObjectData.source = model.getShared(currentIndex)
                }
            }

            DropDown {
                id: smoothingSelect
                model: dataObjectData.smoothingValues
                currentIndex: dataObjectData.smoothing
                Layout.preferredWidth: elementWidth
                ToolTip.visible: hovered
                ToolTip.text: qsTr("frequency smoothing")
                onCurrentIndexChanged: dataObjectData.smoothing = currentIndex
            }

            Item { Layout.fillWidth: true }

            ColorPicker {
                id: colorPicker
                Layout.preferredWidth: 25
                Layout.preferredHeight: 25
                Layout.margins: 5
                onColorChanged: dataObjectData.color = color
                Component.onCompleted: colorPicker.color = dataObjectData.color
            }

            NameField {
                Layout.preferredWidth: 130
                target: dataObject
                Layout.alignment: Qt.AlignVCenter
            }
        }

        // ── Row 2: HP, LP, slope, tap count, export button ───────────────────────
        RowLayout {

            FloatSpinBox {
                id: highpassBox
                from: dataObjectData.minFrequency
                to: 24000
                decimals: 1
                step: 10
                units: "Hz"
                value: dataObjectData.highpass
                onValueChanged: dataObjectData.highpass = value
                tooltiptext: qsTr("highpass cutoff frequency")
                Layout.preferredWidth: elementWidth
                Connections {
                    target: dataObjectData
                    function onHighpassChanged() { highpassBox.value = dataObjectData.highpass; }
                }
            }

            FloatSpinBox {
                id: lowpassBox
                from: 0
                to: 24000
                decimals: 1
                step: 100
                units: "Hz"
                value: dataObjectData.lowpass
                onValueChanged: dataObjectData.lowpass = value
                tooltiptext: qsTr("lowpass cutoff frequency")
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: slopeSelect
                model: ["6 dB/oct", "12 dB/oct", "18 dB/oct", "24 dB/oct", "48 dB/oct"]
                readonly property var slopeValues: [6, 12, 18, 24, 48]
                currentIndex: slopeValues.indexOf(dataObjectData.slope)
                Layout.preferredWidth: elementWidth
                ToolTip.visible: hovered
                ToolTip.text: qsTr("HP/LP rolloff slope")
                onCurrentIndexChanged: dataObjectData.slope = slopeValues[currentIndex]
            }

            DropDown {
                id: tapCountSelect
                model: ["512", "1024"]
                currentIndex: dataObjectData.tapCount === 1024 ? 1 : 0
                Layout.preferredWidth: elementWidth
                ToolTip.visible: hovered
                ToolTip.text: qsTr("number of FIR taps to export")
                onCurrentIndexChanged: dataObjectData.tapCount = (currentIndex === 1 ? 1024 : 512)
                displayText: "Taps: " + currentText
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Export FIR")
                ToolTip.visible: hovered
                ToolTip.text: qsTr("export FIR taps as plain text (.txt) for t.racks FIR DSP 408")
                enabled: dataObjectData.source !== null
                onClicked: firExportDialog.open()
            }
        }
    }

    FileDialog {
        id: firExportDialog
        selectExisting: false
        title: qsTr("Export FIR taps")
        folder: (typeof shortcuts !== 'undefined' ? shortcuts.home : Filesystem.StandardFolder.Home)
        defaultSuffix: "txt"
        onAccepted: dataObjectData.exportFIR(firExportDialog.fileUrl)
    }
}
