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

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.alignment: Qt.AlignTop

            //generator type
            ComboBox {
                id: type
                implicitWidth: 120
                currentIndex: generatorModel.type
                model: generatorModel.types
                onCurrentIndexChanged: generatorModel.type = currentIndex

                ToolTip.visible: hovered
                ToolTip.text: qsTr("signal type")
            }

            SpinBox {
                id: gainSpinBox
                implicitWidth: 180
                value: generatorModel.gain
                from: -90
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

            Rectangle {
                Layout.fillWidth: true
            }

            //Sin frequency
            SpinBox {
                id: frequencySpinBox
                implicitWidth: 180
                visible: type.currentText == 'Sin';
                value: generatorModel.frequency
                from: 20
                to: 20000
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
        }

        RowLayout {
            //list of available output devices:
            ComboBox {
                id: deviceSelect
                model: generatorModel.devices
                displayText: generatorModel.device;

                Layout.fillWidth: true
                currentIndex: model.indexOf(generatorModel.device)
                onCurrentIndexChanged: function () {
                    generatorModel.device = model[currentIndex];
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("audio output device")
            }

            ComboBox {
                implicitWidth: 120
                model: generatorModel.channelsCount
                currentIndex: generatorModel.channel
                onCurrentIndexChanged: generatorModel.channel = currentIndex
                displayText: "ch: " + (currentIndex + 1)
                delegate: ItemDelegate {
                    text: modelData + 1
                    width: parent.width
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("channel number")
            }

            ComboBox {
                implicitWidth: 120
                model: generatorModel.channelsCount
                currentIndex: generatorModel.aux
                onCurrentIndexChanged: generatorModel.aux = currentIndex
                displayText: "aux: " + (currentIndex + 1)
                delegate: ItemDelegate {
                    text: modelData + 1
                    width: parent.width
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("aux channel number")
            }

        }
    }
}
