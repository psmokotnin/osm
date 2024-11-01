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
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.alignment: Qt.AlignTop

            //generator type
            DropDown {
                id: type
                implicitWidth: 140
                currentIndex: generatorModel.type
                model: generatorModel.types
                onCurrentIndexChanged: generatorModel.type = currentIndex

                ToolTip.visible: hovered
                ToolTip.text: qsTr("signal type")
            }

            SelectableSpinBox {
                id: gainSpinBox
                implicitWidth: 165
                value: generatorModel.gain
                from: -140
                to: 0
                editable: true
                onValueChanged: generatorModel.gain = value
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
                value: generatorModel.frequency
                from: 0
                to: 192000
                editable: true
                onValueChanged: generatorModel.frequency = value

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
                value: generatorModel.duration
                from: 0.5
                to: 60
                units: "sec"
                onValueChanged: generatorModel.duration = value
                tooltiptext: qsTr("duration")
            }

            SelectableSpinBox {
                id: sinSweepStart
                implicitWidth: 165
                visible: type.currentText == 'SineSweep';
                value: generatorModel.startFrequency
                from: 20
                to: sinSweepEnd.value-1
                editable: true
                onValueChanged: generatorModel.startFrequency = value

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
                value: generatorModel.endFrequency
                from: sinSweepStart.value+1
                to: 20000
                editable: true
                onValueChanged: generatorModel.endFrequency = value

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

            Label {
                Layout.fillWidth: true
                visible: type.currentText == 'M-Noise™';
                text: qsTr(
                    "<a style='color:%1' href=\"https://m-noise.org/\">M‑Noise</a> is a trademark of Meyer Sound Laboratories <br/>" +
                    "Can be activated if only your audio interface works in 96kHz sample rate."
                ).arg(Material.accentColor)
                onLinkActivated: Qt.openUrlExternally(link)
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignHCenter
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
                checked: generatorModel.evenPolarity
                onCheckedChanged: generatorModel.evenPolarity = checked
                Material.background: parent.Material.background

                ToolTip.visible: hovered
                ToolTip.text: qsTr("inverse polarity at even channels")
            }

            GeneratorChannelSelect {
                id: selectChannels
                tooltip: qsTr("show only selected sources")
                dataObject: generatorModel
                Layout.preferredWidth: 240
            }
        }
    }
}
