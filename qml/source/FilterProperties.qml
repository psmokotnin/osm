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
import OpenSoundMeter 1.0
import Measurement 1.0
import "qrc:/elements"

Item {
    property var dataObject
    readonly property int elementWidth: 200//width / 9

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true

            DropDown {
                id: modeSelect
                model: dataObject.data.modes
                currentIndex: dataObject.data.mode
                displayText: (dataObject.data.mode === Measurement.LFT ? "LTW" : (modeSelect.width > 120 ? "Power:" : "") + currentText)
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Transfrom mode")
                onCurrentIndexChanged: dataObject.data.mode = currentIndex;
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                model: [44100, 48000, 96000, 192000]
                currentIndex: model.indexOf(dataObject.data.sampleRate)
                onCurrentValueChanged: dataObject.data.sampleRate = currentValue;

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
                    dataObject.data.color = color
                }

                Component.onCompleted: {
                    color = dataObject.data.color
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("series color")
            }

            NameField {
                id:titleField
                target: dataObject
                onTextEdited: dataObject.data.autoName = false;
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
                model: dataObject.data.types
                currentIndex: dataObject.data.type
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Filter type")
                onCurrentIndexChanged: {
                    dataObject.data.type = currentIndex;
                    selectOrder.updateModel();
                }
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: selectOrder
                model: dataObject.data.orders
                currentIndex: model.indexOf(dataObject.data.order)
                onCurrentValueChanged: dataObject.data.order = currentValue;

                ToolTip.visible: hovered
                ToolTip.text: qsTr("order")
                Layout.preferredWidth: elementWidth

                function updateModel() {
                    model = dataObject.data.orders;
                }

                Component.onCompleted: updateModel();
            }

            FloatSpinBox {
                value: dataObject.data.cornerFrequency
                from: 1
                to: 96000
                decimals: 1
                step: 1
                units: "Hz"
                onValueChanged: dataObject.data.cornerFrequency = value

                tooltiptext: qsTr("frequency")
                Layout.preferredWidth: elementWidth
            }

            FloatSpinBox {
                value: dataObject.data.gain
                from: -90
                to: 90
                decimals: 1
                step: 0.1
                units: "dB"
                onValueChanged: dataObject.data.gain = value

                visible: dataObject.data.type == 7

                tooltiptext: qsTr("gain")
                Layout.preferredWidth: elementWidth
            }

            FloatSpinBox {
                value: dataObject.data.q
                from: 0.1
                to: 10
                decimals: 1
                step: 0.1
                units: ""
                onValueChanged: dataObject.data.q = value

                visible: dataObject.data.type == 7

                tooltiptext: qsTr("Q")
                Layout.preferredWidth: elementWidth
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
                    var stored = dataObject.data.store();
                    if (stored) {
                        stored.active = true;
                        sourceList.appendItem(stored, true);
                    }
                }
            }
        }
    }
}
