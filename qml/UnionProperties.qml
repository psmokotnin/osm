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
import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.13

import SourceModel 1.0
import OpenSoundMeter 1.0
import "elements"

Item {
    property var dataObject

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            DropDown {
                model: ["Summation", "Subtract", "Average", "Min", "Max", "Apply"]
                currentIndex: dataObject.operation
                onCurrentIndexChanged: {
                    dataObject.operation = currentIndex;
                    if (dataObject.operation === UnionSource.Apply) {
                        dataObject.type = UnionSource.Vector;
                    }
                }
                Layout.preferredWidth: 150

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Math operation")
            }

            DropDown {
                model: ["Vector", "Polar", "dB", "Power"]
                currentIndex: dataObject.type
                onCurrentIndexChanged: dataObject.type = currentIndex;
                enabled: dataObject.operation !== UnionSource.Apply

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Complex numbers behaviour")
            }

            DropDown {
                id: count
                model: [2, 3, 4, 5, 6, 7, 8, 9, 10]
                currentIndex: dataObject.count - 2
                onCurrentIndexChanged: dataObject.count = currentIndex + 2;
                displayText: currentIndex + 2

                ToolTip.visible: hovered
                ToolTip.text: qsTr("count")
            }

            Label {
                text: qsTr("Sources must have the same transform mode")
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
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
                implicitWidth: 120
                Layout.alignment: Qt.AlignVCenter
                onTextEdited: dataObject.autoName = false;
            }

            Button {
                text: qsTr("Store");
                onClicked: {
                    var stored = dataObject.store();
                    stored.active = true;
                    sourceList.appendItem(stored, true);
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("store current measurement")
            }
        }

        RowLayout {

            Repeater {
                id: repeater
                model: count.currentValue

                DropDown {
                    model: SourceModel {
                        id: sourceModel
                        addNone: (modelData > 1 ? true : false)
                        filter: dataObject.uuid
                        noneTitle: "None"
                        list: sourceList
                    }
                    currentIndex: { model.indexOf(dataObject.getSourceId(index)) }
                    property int prevIndex: currentIndex
                    textRole: "title"
                    valueRole: "source"
                    Layout.fillWidth: true
                    onCurrentIndexChanged: {
                        if (!dataObject.setSource(index, model.get(currentIndex))) {
                            currentIndex = prevIndex;
                            return;
                        }
                        prevIndex = currentIndex;
                    }
                }
            }
        }
    }
}
