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

        // ── Row 1: source, smoothing, phase mode, max boost, colour, name ──
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
                Layout.preferredWidth: elementWidth * 1.5
                ToolTip.visible: hovered
                ToolTip.text: qsTr("measurement to correct")
                onCurrentIndexChanged: {
                    dataObjectData.source = model.getShared(currentIndex)
                }
            }

            DropDown {
                id: smoothingSelect
                model: dataObjectData.smoothingValues
                currentIndex: dataObjectData.smoothing
                Layout.preferredWidth: elementWidth * 0.75
                ToolTip.visible: hovered
                ToolTip.text: qsTr("frequency smoothing of the measurement")
                onCurrentIndexChanged: if (currentIndex >= 0) dataObjectData.smoothing = currentIndex
            }

            DropDown {
                id: phaseModeSelect
                model: [qsTr("Linear phase"), qsTr("Minimum phase"), qsTr("Mag + phase")]
                currentIndex: dataObjectData.phaseMode
                Layout.preferredWidth: elementWidth
                ToolTip.visible: hovered
                ToolTip.text: qsTr("linear phase: magnitude only, latency of half the taps\n" +
                                   "minimum phase: magnitude only, no latency\n" +
                                   "mag + phase: magnitude and excess phase, latency of half the taps")
                onCurrentIndexChanged: if (currentIndex >= 0) dataObjectData.phaseMode = currentIndex
            }

            DropDown {
                id: maxBoostSelect
                readonly property var boostValues: [0, 3, 6, 9, 12]
                model: ["+0 dB", "+3 dB", "+6 dB", "+9 dB", "+12 dB"]
                currentIndex: boostValues.indexOf(dataObjectData.maxBoost)
                Layout.preferredWidth: elementWidth * 0.75
                ToolTip.visible: hovered
                ToolTip.text: qsTr("maximum boost of the correction")
                onCurrentIndexChanged: if (currentIndex >= 0) dataObjectData.maxBoost = boostValues[currentIndex]
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

        // ── Row 2: correction range, transition, sample rate, taps, export ──
        RowLayout {

            // lower limit of the correction range ── toggle + frequency
            RowLayout {
                spacing: 0
                Layout.preferredWidth: elementWidth * 1.25

                Button {
                    text: qsTr("From")
                    checkable: true
                    checked: dataObjectData.highpassEnabled
                    flat: true
                    leftPadding: 4
                    rightPadding: 4
                    Material.background: parent.Material.background
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("limit the correction at the low end, otherwise it reaches down to %1 Hz")
                                  .arg(Math.round(dataObjectData.minFrequency))
                    onCheckedChanged: dataObjectData.highpassEnabled = checked
                }

                FloatSpinBox {
                    id: highpassBox
                    Layout.fillWidth: true
                    from: 1
                    to: 96000
                    decimals: 0
                    step: 10
                    units: "Hz"
                    value: dataObjectData.highpass
                    enabled: dataObjectData.highpassEnabled
                    opacity: dataObjectData.highpassEnabled ? 1.0 : 0.4
                    onValueChanged: dataObjectData.highpass = value
                    tooltiptext: qsTr("correction fades out below this frequency, the taps resolve down to %1 Hz")
                                 .arg(Math.round(dataObjectData.minFrequency))
                    Connections {
                        target: dataObjectData
                        function onHighpassChanged() { highpassBox.value = dataObjectData.highpass; }
                    }
                }
            }

            // upper limit of the correction range ── toggle + frequency
            RowLayout {
                spacing: 0
                Layout.preferredWidth: elementWidth * 1.25

                Button {
                    text: qsTr("To")
                    checkable: true
                    checked: dataObjectData.lowpassEnabled
                    flat: true
                    leftPadding: 4
                    rightPadding: 4
                    Material.background: parent.Material.background
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("limit the correction at the high end, otherwise it reaches up to the end of the measurement")
                    onCheckedChanged: dataObjectData.lowpassEnabled = checked
                }

                FloatSpinBox {
                    id: lowpassBox
                    Layout.fillWidth: true
                    from: 1
                    to: 96000
                    decimals: 0
                    step: 100
                    units: "Hz"
                    value: dataObjectData.lowpass
                    enabled: dataObjectData.lowpassEnabled
                    opacity: dataObjectData.lowpassEnabled ? 1.0 : 0.4
                    onValueChanged: dataObjectData.lowpass = value
                    tooltiptext: qsTr("correction fades out above this frequency")
                    Connections {
                        target: dataObjectData
                        function onLowpassChanged() { lowpassBox.value = dataObjectData.lowpass; }
                    }
                }
            }

            DropDown {
                id: transitionSelect
                model: [qsTr("Firm"), qsTr("Hard"), qsTr("Sharp")]
                readonly property var transitionValues: [3, 4, 8]
                currentIndex: transitionValues.indexOf(dataObjectData.transition)
                Layout.preferredWidth: elementWidth * 0.75
                ToolTip.visible: hovered
                ToolTip.text: qsTr("how quickly the correction fades out beyond the limits")
                onCurrentIndexChanged: if (currentIndex >= 0) dataObjectData.transition = transitionValues[currentIndex]
            }

            DropDown {
                id: sampleRateSelect
                readonly property var srValues: [0, 44100, 48000, 88200, 96000, 192000]
                model: dataObjectData.sampleRateValues
                currentIndex: srValues.indexOf(dataObjectData.targetSampleRate)
                Layout.preferredWidth: elementWidth
                ToolTip.visible: hovered
                ToolTip.text: qsTr("sample rate of the target FIR filter")
                onCurrentIndexChanged: if (currentIndex >= 0) dataObjectData.targetSampleRate = srValues[currentIndex]
            }

            DropDown {
                id: tapCountSelect
                readonly property var tapValues: [512, 1024]
                model: ["512", "1024"]
                currentIndex: tapValues.indexOf(dataObjectData.tapCount)
                displayText: "Taps: " + currentText
                Layout.preferredWidth: elementWidth * 0.75
                ToolTip.visible: hovered
                ToolTip.text: qsTr("number of FIR taps, latency %1 ms").arg(dataObjectData.latency.toFixed(2))
                onCurrentIndexChanged: if (currentIndex >= 0) dataObjectData.tapCount = tapValues[currentIndex]
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Export FIR")
                ToolTip.visible: hovered
                ToolTip.text: qsTr("export FIR taps as plain text (.txt) for t.racks FIR DSP 408")
                enabled: dataObjectData.ready
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
        nameFilters: [qsTr("Text files (*.txt)")]
        onAccepted: dataObjectData.exportFIR(firExportDialog.fileUrl)
    }
}
