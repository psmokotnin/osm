/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
import QtQuick.Controls.Material 2.12

import OpenSoundMeter 1.0
import Measurement 1.0
import "qrc:/elements"

Item {
    property var dataObject
    property var dataObjectData : dataObject.data
    readonly property int elementWidth: 200//width / 9

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
                onTextEdited: dataObjectData.autoName = false;
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
                id: typeSelect
                model: dataObjectData.types
                currentIndex: dataObjectData.type
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Filter type")
                onCurrentIndexChanged: {
                    dataObjectData.type = currentIndex;
                    selectOrder.updateModel();
                }
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: selectOrder
                model: dataObjectData.orders
                currentIndex: model.indexOf(dataObjectData.order)
                onCurrentValueChanged: dataObjectData.order = currentValue;

                ToolTip.visible: hovered
                ToolTip.text: qsTr("order")
                Layout.preferredWidth: elementWidth

                function updateModel() {
                    model = dataObjectData.orders;
                }

                Component.onCompleted: updateModel();
            }

            FloatSpinBox {
                value: dataObjectData.cornerFrequency
                from: 1
                to: 96000
                decimals: 1
                step: 1
                units: "Hz"
                onValueChanged: dataObjectData.cornerFrequency = value

                tooltiptext: qsTr("frequency")
                Layout.preferredWidth: elementWidth
            }

            FloatSpinBox {
                value: dataObjectData.gain
                from: -90
                to: 90
                decimals: 1
                step: 0.1
                units: "dB"
                onValueChanged: dataObjectData.gain = value

                visible: dataObjectData.gainAvailable

                tooltiptext: qsTr("gain")
                Layout.preferredWidth: elementWidth
            }

            FloatSpinBox {
                value: dataObjectData.q
                from: 0.1
                to: 20
                decimals: 1
                step: 0.1
                units: ""
                onValueChanged: dataObjectData.q = value

                visible: dataObjectData.qAvailable

                tooltiptext: qsTr("Q")
                Layout.preferredWidth: elementWidth
            }

            Button {
                text: "+/–"
                checkable: true
                checked: dataObjectData.polarity
                onCheckedChanged: dataObjectData.polarity = checked

                Material.background: parent.Material.background

                ToolTip.visible: hovered
                ToolTip.text: qsTr("inverse polarity")
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Store");
                ToolTip.visible: hovered
                ToolTip.text: qsTr("store current measurement")
                implicitWidth: 75

                onClicked: {
                    var stored = dataObjectData.store();
                    if (stored) {
                        stored.data.active = true;
                        sourceList.appendItem(stored, true);
                    }
                }
            }
        }
    }
}
