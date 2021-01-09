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

            Shortcut {
                sequence: "Ctrl+1"
                onActivated: chartsCount.currentIndex = 0;
            }
            Shortcut {
                sequence: "Ctrl+2"
                onActivated: chartsCount.currentIndex = 1;
            }
            Shortcut {
                sequence: "Ctrl+3"
                onActivated: chartsCount.currentIndex = 2;
            }
        }

        Generator {}

        Component {
            id: measurementDelegate
            Measurement {
                id: item
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: storedDelegate
            Stored {
                id: item
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: unionDelegate
            Union {
                id: item
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: elcDelegate
            ELC {
                id: item
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
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

                    anchors {
                        left: parent ? parent.left : null
                        right: parent ? parent.right : null
                    }
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
                    onClicked: {
                        if (sideList.currentIndex != index) {
                            sideList.currentIndex = index;
                            sideList.forceActiveFocus();
                        } else {
                            sideList.currentIndex = -1
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
                            property bool modelHighlight: index == sideList.currentIndex
                            sourceComponent:
                                switch(model.name) {
                                    case "Measurement": return measurementDelegate;
                                    case "Stored": return storedDelegate;
                                    case "Union": return unionDelegate;
                                    case "ELC": return elcDelegate;
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
                            background: Rectangle {
                                color: "transparent"
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
            onCurrentIndexChanged: sourceList.selectedIndex = currentIndex
            currentIndex: -1
            highlight: Rectangle {
                border.color: sourceList.selected ? sourceList.selected.color : "black"
                border.width: 0.5
                color: "transparent"
            }

            Shortcut {
                sequence: "Ctrl+X"
                context: Qt.ApplicationShortcut
                onActivated: {
                    for (var i = 0; i < sourceList.count; i++) {
                        if (sourceList.get(i).objectName === "Measurement"){
                            var stored = sourceList.get(i).store();
                            stored.name = 'Stored #' + (sourceList.count - 0);
                            stored.active = true;
                            sourceList.appendItem(stored, true);
                        }
                    }
                }
            }

            Shortcut {
                sequence: "Ctrl+R"
                context: Qt.ApplicationShortcut
                onActivated: {
                    for (var i = 0; i < sourceList.count; i++) {
                        if (sourceList.get(i).objectName === "Measurement"){
                            sourceList.get(i).resetAverage();
                        }
                    }
                }
            }
        }

        RowLayout {
            spacing: 5
            Layout.bottomMargin: 8

            Button {
                Layout.fillWidth: true
                font.family: "Osm"
                text: "\uf09a follow"
                Material.foreground: Material.Indigo
                flat: true
                font.pixelSize: 10
                onClicked: {
                    Qt.openUrlExternally("https://www.facebook.com/opensoundmeter/");
                }
            }

            Button {
                Layout.fillWidth: true
                font.family: "Osm"
                text: "\ue800 support"
                Material.foreground: Material.Pink
                flat: true
                font.pixelSize: 10
                onClicked: {
                    Qt.openUrlExternally("https://opensoundmeter.com/support");
                }
            }
        }
    }
}
