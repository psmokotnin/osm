/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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
import QtQuick.Layouts 1.13
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.12

import OpenSoundMeter 1.0
import SourceModel 1.0
import "../elements"

Item {
    property var dataObject
    readonly property int elementWidth: 200//width / 9

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            FloatSpinBox {
                id: wideSpinBox
                Layout.preferredWidth: elementWidth
                value: dataObject.wide
                from: -2000
                to: 2000
                units: "ms"
                onValueChanged: dataObject.wide = value
                tooltiptext: qsTr("Wide of Tukey window, ms")
                Layout.alignment: Qt.AlignVCenter
                Connections {
                    target: dataObject
                    function onWideChanged() {
                        wideSpinBox.value = dataObject.wide;
                    }
                }
            }

            FloatSpinBox {
                id: offsetSpinBox
                Layout.preferredWidth: elementWidth
                value: dataObject.offset
                from: -2000
                to: 2000
                units: "ms"
                onValueChanged: dataObject.offset = value
                tooltiptext: qsTr("offset zero point, ms")
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }

            ColorPicker {
                id: colorPicker

                Layout.preferredWidth: 25
                Layout.preferredHeight: 25
                Layout.margins: 5

                onColorChanged: {
                    dataObject.color = color
                }
                Component.onCompleted: {
                    colorPicker.color = dataObject.color
                }
            }

            NameField {
                Layout.preferredWidth: 130
                target: dataObject
                Layout.alignment: Qt.AlignVCenter
            }
        }

        RowLayout {

            DropDown {
                id: modeSelect
                model: dataObject.modes
                currentIndex: dataObject.mode
                displayText: ( (modeSelect.width > 120 ? "Power:" : "") + currentText)
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
                model: SourceModel {
                    id: sourceModel
                    addNone: false
                    filter: dataObject.uuid
                    noneTitle: "None"
                    list: sourceList
                }
                currentIndex: {
                    model.indexOf(dataObject.source)
                }
                textRole: "title"
                valueRole: "source"
                Layout.preferredWidth: elementWidth
                onCurrentIndexChanged: {
                    dataObject.source = model.get(currentIndex);
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Store");
                onClicked: {
                    var stored = dataObject.store();
                    stored.active = true;
                    sourceList.appendItem(stored, true);
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("store current result")
            }
        }
    }
}
