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
import "qrc:/elements"

Item {
    property var dataObject
    readonly property int elementWidth: width / 5

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            DropDown {
                id: domainSelect
                model: ["Time", "Frequency"]
                currentIndex: dataObject.domain
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Take data from")
                onCurrentIndexChanged: dataObject.domain = currentIndex;
                Layout.preferredWidth: elementWidth
            }

            FloatSpinBox {
                id: wideSpinBox
                from: 0.1
                to: 10000
                units: "ms"
                value: dataObject.wide
                property bool completed: false
                onValueChanged: {if (completed) { dataObject.wide = value; } }
                tooltiptext: qsTr("Wide of Tukey window, ms")
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: elementWidth
                visible: dataObject.domain === 0
                Connections {
                    target: dataObject
                    function onWideChanged() {
                        wideSpinBox.value = dataObject.wide;
                    }
                }
                Component.onCompleted: {
                    completed = true;
                    wideSpinBox.value = dataObject.wide;
                }
            }

            FloatSpinBox {
                id: offsetSpinBox
                from: -2000
                to: 2000
                units: "ms"
                value: dataObject.offset
                onValueChanged: dataObject.offset = value
                tooltiptext: qsTr("offset zero point, ms")
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: elementWidth
                visible: dataObject.domain === 0
            }

            FloatSpinBox {
                value: dataObject.minFrequency
                from: 0
                to: 96000
                decimals: 1
                step: 1
                units: "Hz"
                onValueChanged: dataObject.minFrequency = value

                tooltiptext: qsTr("min frequency")
                Layout.preferredWidth: elementWidth

                visible: dataObject.domain === 1
            }

            FloatSpinBox {
                value: dataObject.maxFrequency
                from: 0
                to: 96000
                decimals: 1
                step: 1
                units: "Hz"
                onValueChanged: dataObject.maxFrequency = value

                tooltiptext: qsTr("max frequency")
                Layout.preferredWidth: elementWidth

                visible: dataObject.domain === 1
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

            DropDown {
                id: modeSelect
                model:[
                    { text: "FFT 8",  enabled: true },
                    { text: "FFT 9",  enabled: true },
                    { text: "FFT 10", enabled: true },
                    { text: "FFT 11", enabled: true },
                    { text: "FFT 12", enabled: true },
                    { text: "FFT 13", enabled: true },
                    { text: "FFT 14", enabled: true },
                    { text: "FFT 15", enabled: true },
                    { text: "FFT 16", enabled: true },

                    { text: "LTW 1", enabled: dataObject.domain === 0 },
                    { text: "LTW 2", enabled: dataObject.domain === 0 },
                    { text: "LTW 3", enabled: dataObject.domain === 0 },
                ]
                textRole: "text"
                delegate: MenuItem {
                    width: ListView.view.width
                    text: modelData[modeSelect.textRole]
                    enabled: modelData["enabled"]
                    Material.foreground: modeSelect.currentIndex === index ? ListView.view.contentItem.Material.accent : ListView.view.contentItem.Material.foreground
                    highlighted: modeSelect.highlightedIndex === index
                    hoverEnabled: modeSelect.hoverEnabled
                }

                currentIndex: dataObject.mode
                onCurrentIndexChanged: dataObject.mode = currentIndex;
                Layout.preferredWidth: elementWidth
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Transfrom mode")
            }

            DropDown {
                id: windowSelect
                model: dataObject.windows
                currentIndex: dataObject.window
                onCurrentIndexChanged: dataObject.window = currentIndex
                visible: dataObject.domain === 0
                Layout.preferredWidth: elementWidth
                ToolTip.visible: hovered
                ToolTip.text: qsTr("window function")
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
