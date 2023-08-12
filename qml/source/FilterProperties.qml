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
                model: dataObject.modes
                currentIndex: dataObject.mode
                displayText: (dataObject.mode === Measurement.LFT ? "LTW" : (modeSelect.width > 120 ? "Power:" : "") + currentText)
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Transfrom mode")
                onCurrentIndexChanged: dataObject.mode = currentIndex;
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                model: [44100, 48000, 96000, 192000]
                currentIndex: model.indexOf(dataObject.sampleRate)
                onCurrentValueChanged: dataObject.sampleRate = currentValue;

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
                onTextEdited: dataObject.autoName = false;
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
                model: dataObject.types
                currentIndex: dataObject.type
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Filter type")
                onCurrentIndexChanged: {
                    dataObject.type = currentIndex;
                    selectOrder.updateModel();
                }
                Layout.preferredWidth: elementWidth
            }

            DropDown {
                id: selectOrder
                model: dataObject.orders
                currentIndex: model.indexOf(dataObject.order)
                onCurrentValueChanged: dataObject.order = currentValue;

                ToolTip.visible: hovered
                ToolTip.text: qsTr("order")
                Layout.preferredWidth: elementWidth

                function updateModel() {
                    model = dataObject.orders;
                }

                Component.onCompleted: updateModel();
            }

            FloatSpinBox {
                value: dataObject.cornerFrequency
                from: 1
                to: 96000
                decimals: 1
                step: 1
                units: "Hz"
                onValueChanged: dataObject.cornerFrequency = value

                tooltiptext: qsTr("frequency")
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
                    var stored = dataObject.store();
                    if (stored) {
                        stored.active = true;
                        sourceList.appendItem(stored, true);
                    }
                }
            }
        }
    }
}
