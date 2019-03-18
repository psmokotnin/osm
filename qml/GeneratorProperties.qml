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
        spacing: 10

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
                model: generatorModel.chanelsCount
                currentIndex: generatorModel.chanel
                onCurrentIndexChanged: generatorModel.chanel = currentIndex
                displayText: "chanel: " + (currentIndex + 1)
                delegate: ItemDelegate {
                          text: modelData + 1
                          width: parent.width
                      }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("chanel number")
            }

            ComboBox {
                model: generatorModel.chanelsCount
                currentIndex: generatorModel.aux
                onCurrentIndexChanged: generatorModel.aux = currentIndex
                displayText: "aux: " + (currentIndex + 1)
                delegate: ItemDelegate {
                          text: modelData + 1
                          width: parent.width
                      }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("aux chanel number")
            }

        }
        RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignTop

            SpinBox {
                id: gainSpinBox
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

            //generator type
            ComboBox {
                id: type
                currentIndex: generatorModel.type
                model: generatorModel.types
                onCurrentIndexChanged: generatorModel.type = currentIndex

                ToolTip.visible: hovered
                ToolTip.text: qsTr("signal type")
            }

            //Sin frequency
            SpinBox {
                visible: type.currentText == 'Sin';
                id: frequencySpinBox
                value: generatorModel.frequency
                from: 20
                to: 20000
                editable: true
                onValueChanged: generatorModel.frequency = value

                ToolTip.visible: hovered
                ToolTip.text: qsTr("frequency for sin")
            }

            SpinBox{
                property int decimals: 2

                id: gainSpinBox

                validator: DoubleValidator {
                    bottom: Math.min(gainSpinBox.from, gainSpinBox.to)
                    top:  Math.max(gainSpinBox.from, gainSpinBox.to)
                }

                textFromValue: function(value, locale) {
                    return Number(value / 100).toLocaleString(locale, 'f', gainSpinBox.decimals)
                }

                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text) * 100
                }

                from: 0
                to: 100
                stepSize: 1
                editable: true
                onValueChanged: generatorModel.gain = value / 100

                Component.onCompleted: value = generatorModel.gain * 100

                ToolTip.visible: hovered
                ToolTip.text: qsTr("output gain")
            }
        }
    }
}
