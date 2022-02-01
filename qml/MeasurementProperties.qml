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
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.13

import OpenSoundMeter 1.0
import Measurement 1.0
import Audio 1.0
import "elements"

Item {
    id: measurementProperties
    property var dataObject
    readonly property int elementWidth: width / 7.7
    readonly property int spinboxWidth: width / 11

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            DropDown {
                id: averageType
                Layout.preferredWidth: elementWidth
                model: ["off", "LPF", "FIFO"]
                currentIndex: dataObject.averageType
                ToolTip.visible: hovered
                ToolTip.text: qsTr("average type")
                onCurrentIndexChanged: dataObject.averageType = currentIndex;
            }

            SelectableSpinBox {
                Layout.preferredWidth: elementWidth
                value: dataObject.average
                from: 1
                to: 100
                editable: true
                onValueChanged: dataObject.average = value

                ToolTip.visible: hovered
                ToolTip.text: qsTr("average count")

                visible: dataObject.averageType === Measurement.FIFO;
            }

            DropDown {
                Layout.preferredWidth: elementWidth
                model: [ "0.25Hz", "0.5Hz", "1Hz" ]
                currentIndex: dataObject.filtersFrequency
                onCurrentIndexChanged: dataObject.filtersFrequency = currentIndex;

                ToolTip.visible: hovered
                ToolTip.text: qsTr("LPF frequency")

                visible: dataObject.averageType === Measurement.LPF;
            }

            Rectangle {
                Layout.preferredWidth: elementWidth
                visible: dataObject.averageType === Measurement.OFF;
            }

            Button {
                text: "+/–"
                checkable: true
                checked: dataObject.polarity
                onCheckedChanged: dataObject.polarity = checked
                Layout.preferredWidth: (elementWidth - 5) / 2
                Material.background: parent.Material.background

                ToolTip.visible: hovered
                ToolTip.text: qsTr("inverse polarity at measurement chanel")
            }

            Button {
                font.family: "Osm"
                text: "\ue808"
                onClicked: dataObject.resetAverage()
                Layout.preferredWidth: (elementWidth - 5) / 2
                Material.background: parent.Material.background

                ToolTip.visible: hovered
                ToolTip.text: qsTr("reset buffers")
            }

            Item {
                id: calibartionGroup
                Layout.preferredWidth: elementWidth
                Layout.fillHeight: true

                CheckBox {
                    id: calibrateOn
                    text: qsTr("calibrate")
                    Layout.maximumWidth: elementWidth - 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    checked: dataObject.calibration
                    onCheckStateChanged: {
                        if (checked) {
                            if (dataObject.calibrationLoaded) {
                                dataObject.calibration = checked;
                            } else {
                                openCalibrationFileDialog.open();
                            }
                        } else {
                            dataObject.calibration = false;
                        }
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("apply calibration")

                    contentItem: Text {
                        leftPadding: calibrateOn.indicator && !calibrateOn.mirrored ? calibrateOn.indicator.width + calibrateOn.spacing : 0
                        rightPadding: calibrateOn.indicator && calibrateOn.mirrored ? calibrateOn.indicator.width + calibrateOn.spacing : 0
                        text: calibrateOn.text
                        font: calibrateOn.font
                        color: calibrateOn.enabled ? calibrateOn.Material.foreground : calibrateOn.Material.hintTextColor
                        elide: Text.ElideNone
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Button {
                    implicitWidth: 30
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: calibrateOn.right
                    anchors.leftMargin: -10
                    flat: true
                    spacing: 0
                    text: "..."
                    onClicked: {openCalibrationFileDialog.open();}
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("open calibration file")
                }
                FileDialog {
                    id: openCalibrationFileDialog
                    selectExisting: true
                    title: qsTr("Please choose a file's name")
                    folder: (typeof shortcuts !== 'undefined' ? shortcuts.home : Filesystem.StandardFolder.Home)
                    onAccepted: function() {
                        if (dataObject.loadCalibrationFile(openCalibrationFileDialog.fileUrl)) {
                            dataObject.calibration = true;
                        } else {
                            dataObject.calibration = false;
                        }
                    }
                    onRejected: {
                        dataObject.calibration = false;
                        calibrateOn.checked = dataObject.calibration;
                    }
                }
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

            NameField {
                id:titleField
                target: dataObject
                Layout.preferredWidth: elementWidth - 25
                Layout.minimumWidth: 100
                Layout.alignment: Qt.AlignVCenter
            }

            RowLayout {
                Layout.fillWidth: true
            }

            FloatSpinBox {
                id: gainSpinBox
                Layout.preferredWidth: spinboxWidth
                value: dataObject.gain
                from: -90
                to: 20
                units: "dB"
                indicators: false
                onValueChanged: dataObject.gain = value
                tooltiptext: qsTr("gain")
                implicitHeight: titleField.implicitHeight
                Layout.alignment: Qt.AlignVCenter
            }

            SelectableSpinBox {
                id: delaySpin
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: spinboxWidth
                value: dataObject.delay
                implicitHeight: titleField.implicitHeight
                from: 0
                to: 48000
                editable: true
                spacing: 0
                down.indicator.width: 0
                up.indicator.width: 0
                onValueChanged: dataObject.delay = value

                textFromValue: function(value, locale) {
                    return Number(1000 * value / dataObject.sampleRate).toLocaleString(locale, 'f', 2) + "ms";
                }

                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text.replace("ms", "")) * dataObject.sampleRate / 1000;
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("estimated delay delta: <b>%L1ms</b>")
                    .arg(Number(1000 * dataObject.estimatedDelta / dataObject.sampleRate).toLocaleString(locale, 'f', 2));
            }

            Button {
                text: qsTr("%L1 ms")
                    .arg(Number(1000 * dataObject.estimated / dataObject.sampleRate).toLocaleString(locale, 'f', 2));
                onClicked: {
                    delaySpin.value = dataObject.estimated;
                }
                implicitWidth: 75

                font.capitalization: Font.AllLowercase
                ToolTip.visible: hovered
                ToolTip.text: qsTr("apply estimated delay")
            }
        }

        RowLayout {
            Layout.fillWidth: true

            DropDown {
                id: modeSelect
                model: dataObject.modes
                currentIndex: dataObject.mode
                displayText: (dataObject.mode === Measurement.LFT ? "LTW" : (modeSelect.width > 120 ? "Power:" : "") + currentText)
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Transfrom mode")
                onCurrentIndexChanged: dataObject.mode = currentIndex;
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: windowSelect
                model: dataObject.windows
                currentIndex: dataObject.window
                onCurrentIndexChanged: dataObject.window = currentIndex
                ToolTip.visible: hovered
                ToolTip.text: qsTr("window function")
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: measurementChannel
                currentIndex: dataObject.dataChanel
                onCurrentIndexChanged: dataObject.dataChanel = currentIndex
                displayText: "M: " + currentText
                ToolTip.visible: hovered
                ToolTip.text: qsTr("measurement chanel number")
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: referenceChannel
                currentIndex: dataObject.referenceChanel
                onCurrentIndexChanged: dataObject.referenceChanel = currentIndex
                displayText: "R: " + currentText
                ToolTip.visible: hovered
                ToolTip.text: qsTr("reference chanel number")
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: deviceSelect
                Layout.fillWidth: true
                model: DeviceModel {
                    id: deviceModel
                    scope: DeviceModel.InputOnly
                }
                textRole: "name"
                valueRole: "id"
                currentIndex: { model.indexOf(dataObject.deviceId) }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("audio input device")
                onCurrentIndexChanged: {
                    var measurementIndex = measurementChannel.currentIndex;
                    var referenceIndex = referenceChannel.currentIndex;
                    var channelNames = deviceModel.channelNames(deviceSelect.currentIndex);
                    channelNames.push("Loop");
                    dataObject.deviceId = model.deviceId(currentIndex);
                    measurementChannel.model = channelNames;
                    referenceChannel.model   = channelNames;

                    measurementChannel.currentIndex = measurementIndex < channelNames.length + 1 ? measurementIndex : -1;
                    referenceChannel.currentIndex = referenceIndex < channelNames.length + 1 ? referenceIndex : -1;
                }

                Connections {
                    target: deviceModel
                    function onModelReset() {
                        deviceSelect.currentIndex = deviceModel.indexOf(dataObject.deviceId);
                        var measurementIndex = measurementChannel.currentIndex;
                        var referenceIndex = referenceChannel.currentIndex;
                        var channelNames = deviceModel.channelNames(deviceSelect.currentIndex);
                        channelNames.push("Loop");

                        measurementChannel.model = channelNames;
                        referenceChannel.model   = channelNames;

                        measurementChannel.currentIndex = measurementIndex < channelNames.length + 1 ? measurementIndex : -1;
                        referenceChannel.currentIndex = referenceIndex < channelNames.length + 1 ? referenceIndex : -1;
                    }
                }
            }

            Button {
                text: qsTr("Store");
                onClicked: measurementProperties.store()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("store current measurement")
                implicitWidth: 75
            }

            Shortcut {
                sequence: "Ctrl+C"
                onActivated: measurementProperties.store()
            }

            Shortcut {
                sequence: "Ctrl+E"
                onActivated: {
                    delaySpin.value = dataObject.estimated;
                }
            }
        }
    }//ColumnLayout

    function store() {
        var stored = dataObject.store();
        stored.active = true;
        sourceList.appendItem(stored, true);
    }
}
