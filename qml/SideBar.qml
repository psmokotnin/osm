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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQml 2.2
import QtQml.Models 2.3
import QtQuick.Controls.Material 2.12

import SourceModel 1.0
/**
 * SideBar
 *
 * There are all current active measurements, stores and sound sources
 * at the sidebar item.
 *
 * For detail settings the bottom bar is used.
 */
Item {
    property Item list : sideList
    property int colorIndex: 6;

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        ComboBox {
            id: chartsCount
            Layout.alignment: Qt.AlignHCenter
            model: ["Single", "Double", "Three"]
            currentIndex: charts.count - 1
            onCurrentIndexChanged: {
                applicationWindow.charts.count = currentIndex + 1
            }
        }

        Generator {}

        Component {
            id: measurementDelegate
            Measurement {
                id: item
                width: sideList.width
                dataModel: modelData
            }
        }
        Component {
            id: storedDelegate
            Stored {
                id: item
                width: sideList.width
                dataModel: modelData
            }
        }

        ListView {
            id: sideList
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.margins: 5
            model: SourceModel {
                id: sourceModel
                list: sourceList
            }
            clip: true
            delegate: Component {

                MouseArea {
                    id: dragArea
                    property bool held: false

                    anchors { left: parent.left; right: parent.right }
                    height: content.height

                    drag.target:    dragArea.held ? content : undefined
                    drag.axis:      Drag.YAxis
                    cursorShape:    Qt.PointingHandCursor

                    onPressAndHold: dragArea.held = true;
                    onReleased: {
                        //onClick open properties
                        if (
                                loaded.status == Loader.Ready &&
                                loaded.item.dataModel &&
                                loaded.item.propertiesQml
                           ) {
                            applicationWindow.properiesbar.open(loaded.item.dataModel, loaded.item.propertiesQml);
                        }
                        //release drag
                        if (dragArea.held) {
                            dragArea.held = false;
                            sourceModel.layoutChanged();
                        }
                    }

                    Item {
                        id: content
                        anchors { left: parent.left; right: parent.right }
                        height: loaded.height
                        Drag.active: dragArea.held
                        Drag.source: dragArea
                        Drag.hotSpot.x: width / 2
                        Drag.hotSpot.y: height / 2

                        Loader {
                            id: loaded
                            property var modelData: model.source
                            sourceComponent:
                                switch(model.name) {
                                    case "Measurement": return measurementDelegate;
                                    case "Stored": return storedDelegate;
                                    default: console.log("unknow model " + model.name);return ;
                            }
                        }

                        states: State {
                            when: dragArea.held

                            ParentChange { target: content; parent: applicationWindow.dataSourceList }
                            AnchorChanges {
                                target: content
                                anchors { horizontalCenter: undefined; verticalCenter: undefined }
                            }
                        }

                        Button {
                            font.family: "Osm"
                            text: "\ue801"
                            anchors.right: parent.right
                            anchors.top: parent.top
                            flat: true
                            font.pixelSize: 16
                            onClicked: {
                                applicationWindow.dialog.accepted.connect(deleteModel);
                                applicationWindow.dialog.rejected.connect(freeDialog);
                                applicationWindow.dialog.title = "Delete " + model.source.name + "?";
                                applicationWindow.dialog.open();
                            }
                            function deleteModel() {
                                if (applicationWindow.properiesbar.currentObject === model.source) {
                                    applicationWindow.properiesbar.reset();
                                }
                                sourceList.removeItem(model.source);
                                freeDialog();
                            }
                            function freeDialog() {
                                applicationWindow.dialog.accepted.disconnect(deleteModel);
                                applicationWindow.dialog.rejected.disconnect(freeDialog);
                            }
                        }
                    }

                    DropArea {
                        anchors { fill: parent; margins: 0 }
                        onEntered: {
                            sourceList.move(
                                    drag.source.DelegateModel.itemsIndex,
                                    dragArea.DelegateModel.itemsIndex)
                        }
                    }
                }
            }
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
}
