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
import QtQuick.Layouts 1.1

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        //list of available output devices:
        ComboBox {
            id: deviceSelect
            model: generatorModel.devices
            implicitWidth: parent.width
            currentIndex: { model.indexOf(generatorModel.device) }
            onCurrentIndexChanged: generatorModel.device = model[currentIndex]

            ToolTip.visible: hovered
            ToolTip.text: qsTr("audio output device")

            displayText: generatorModel.device;
        }

        RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignTop

            //generator type
            ComboBox {
                currentIndex: generatorModel.type
                model: generatorModel.types
                onCurrentIndexChanged: generatorModel.type = currentIndex

                ToolTip.visible: hovered
                ToolTip.text: qsTr("signal type")
            }

            //Sin frequency
            SpinBox {
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
