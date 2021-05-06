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
import "elements"

Item {
    property var dataObject

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            DropDown {
                model: ["Sum", "Diff", "Avg"]
                currentIndex: dataObject.operation
                onCurrentIndexChanged: dataObject.operation = currentIndex;
            }

            DropDown {
                model: ["Vector", "Polar"]
                currentIndex: dataObject.type
                onCurrentIndexChanged: dataObject.type = currentIndex;
            }

            Label {
                text: qsTr("Sources must have the same size")
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
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
                implicitWidth: 100
                Layout.alignment: Qt.AlignVCenter
            }

            Button {
                text: qsTr("Store");
                onClicked: {
                    var stored = dataObject.store();
                    stored.name = 'Stored #' + (sourceList.count - 0);
                    stored.active = true;
                    sourceList.appendItem(stored, true);
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("store current measurement")
            }
        }

        RowLayout {

            DropDown {
                model: SourceModel {
                    id: sourceModel
                    filter: true
                    list: sourceList
                }
                currentIndex: { model.indexOf(dataObject.getSource(0)) }
                textRole: "title"
                valueRole: "source"
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    dataObject.setSource(0, model.get(currentIndex));
                }
            }

            DropDown {
                model: SourceModel {
                    filter: true
                    list: sourceList
                }
                currentIndex: { model.indexOf(dataObject.getSource(1)) }
                textRole: "title"
                valueRole: "source"
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    dataObject.setSource(1, model.get(currentIndex));
                }
            }

            DropDown {
                model: SourceModel {
                    filter: true
                    addNone: true
                    list: sourceList
                }
                currentIndex: { model.indexOf(dataObject.getSource(2)) }
                textRole: "title"
                valueRole: "source"
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    dataObject.setSource(2, model.get(currentIndex));
                }
            }

            DropDown {
                model: SourceModel {
                    filter: true
                    addNone: true
                    list: sourceList
                }
                currentIndex: { model.indexOf(dataObject.getSource(3)) }
                textRole: "title"
                valueRole: "source"
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    dataObject.setSource(3, model.get(currentIndex));
                }
            }

        }
    }
}
