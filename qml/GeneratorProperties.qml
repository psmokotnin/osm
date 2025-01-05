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
import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import OpenSoundMeterModule 1.0
import Audio 1.0
import "elements"

Item {
    id: control
    property var currentGenerator : (remoteClient.controlledGenerator && remoteClient.controlledGenerator.data ? remoteClient.controlledGenerator.data : generatorModel)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.alignment: Qt.AlignTop

            //generator type
            DropDown {
                id: type
                implicitWidth: 140
                currentIndex: control.currentGenerator.type
                model: generatorModel.types
                onCurrentIndexChanged: control.currentGenerator.type = currentIndex

                ToolTip.visible: hovered
                ToolTip.text: qsTr("signal type")
            }

            SelectableSpinBox {
                id: gainSpinBox
                implicitWidth: 165
                value: control.currentGenerator.gain
                from: -140
                to: 0
                editable: true
                onValueChanged: control.currentGenerator.gain = value
                textFromValue: function(value, locale) {
                    return Number(value) + "dB"
                }

                valueFromText: function(text, locale) {
                    return parseInt(text)
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("gain")
            }

            //Sin frequency
            SelectableSpinBox {
                id: frequencySpinBox
                implicitWidth: 165
                visible: type.currentText == 'Sin' || type.currentText == 'SinBurst'
                value: control.currentGenerator.frequency
                from: 0
                to: 192000
                editable: true
                onValueChanged: control.currentGenerator.frequency = value

                textFromValue: function(value, locale) {
                    return Number(value) + "Hz"
                }

                valueFromText: function(text, locale) {
                    return parseInt(text)
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("frequency for sin")
            }

            //Sin Sweep from
            FloatSpinBox {
                //id: sinSweepDuration
                visible: type.currentText == 'SineSweep';
                implicitWidth: 165
                value: control.currentGenerator.duration
                from: 0.5
                to: 60
                units: "sec"
                onValueChanged: control.currentGenerator.duration = value
                tooltiptext: qsTr("duration")
            }

            SelectableSpinBox {
                id: sinSweepStart
                implicitWidth: 165
                visible: type.currentText == 'SineSweep';
                value: control.currentGenerator.startFrequency
                from: 20
                to: sinSweepEnd.value-1
                editable: true
                onValueChanged: control.currentGenerator.startFrequency = value

                textFromValue: function(value, locale) {
                    return Number(value) + "Hz"
                }

                valueFromText: function(text, locale) {
                    return parseInt(text)
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Start frequency")
            }

            //Sin Sweep to
            SelectableSpinBox {
                id: sinSweepEnd
                implicitWidth: 165
                visible: type.currentText == 'SineSweep';
                value: control.currentGenerator.endFrequency
                from: sinSweepStart.value+1
                to: 20000
                editable: true
                onValueChanged: control.currentGenerator.endFrequency = value

                textFromValue: function(value, locale) {
                    return Number(value) + "Hz"
                }

                valueFromText: function(text, locale) {
                    return parseInt(text)
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("End frequency")
            }

            Button {
                id: octaveDown
                implicitWidth: 120
                text: qsTr("Octave Down")
                visible: type.currentText == 'Sin';
                enabled: frequencySpinBox.value/2 >= frequencySpinBox.from
                onClicked: frequencySpinBox.value /= 2
            }

            Button {
                id:octaveUp
                implicitWidth: 120
                text: qsTr("Octave Up")
                visible: type.currentText == 'Sin';
                enabled: frequencySpinBox.value*2 <= frequencySpinBox.to
                onClicked: frequencySpinBox.value *= 2
            }

            Item {
                Layout.fillWidth: true
            }

            DropDown {
                id: selectTarget
                enabled: !control.currentGenerator.enabled
                model: remoteClient.generatorsList
                currentIndex: remoteClient.generatorsList.indexOf(
                              remoteClient.controlledGenerator && remoteClient.controlledGenerator.data ? remoteClient.controlledGenerator.data.host : "Local"
                            )
                onCurrentValueChanged: remoteClient.selectGenerator(currentValue)

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Controlled generator")

                Layout.preferredWidth: 240
            }
        }

        RowLayout {
            DropDown {
                id: deviceSelect
                model: DeviceModel {
                    id: deviceModel
                    scope: DeviceModel.OutputOnly
                }
                textRole: "name"
                valueRole: "id"
                enabled: selectTarget.currentIndex === 0

                Layout.fillWidth: true
                currentIndex: model.indexOf(generatorModel.deviceId)
                onCurrentIndexChanged: function () {
                    deviceSelect.updateChannelNames();
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("audio output device")
                Connections {
                    target: deviceModel
                    function onModelReset() {
                        deviceSelect.updateChannelNames();
                    }
                }

                function updateChannelNames() {
                    selectChannels.list = model.channelNames(currentIndex);
                    generatorModel.deviceId = model.deviceId(currentIndex);
                }
            }

            Button {
                text: "even inv"
                checkable: true
                enabled: selectTarget.currentIndex === 0
                checked: generatorModel.evenPolarity
                onCheckedChanged: control.currentGenerator.evenPolarity = checked
                Material.background: parent.Material.background

                ToolTip.visible: hovered
                ToolTip.text: qsTr("inverse polarity at even channels")
            }

            GeneratorChannelSelect {
                id: selectChannels
                tooltip: qsTr("show only selected sources")
                dataObject: generatorModel
                enabled: selectTarget.currentIndex === 0
                Layout.preferredWidth: 240
            }
        }
    }
}
