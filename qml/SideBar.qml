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
import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQml 2.2
import QtQml.Models 2.3
import QtQuick.Controls.Material 2.12

import SourceModel 1.0
import OpenSoundMeter 1.0
import "elements"
import "source"

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

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            DropDown {
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
                Shortcut {
                    sequence: "Ctrl+4"
                    onActivated: {
                        applicationWindow.charts.autoHeight();
                    }
                }
            }
            Button {
                font.family: "Osm"
                font.pixelSize: 30
                text: "\uf0C9"
                flat: true
                onClicked: applicationWindow.sideMenu.open();
                visible: applicationAppearance.showMenuBar ? false : true
            }
        }

        Generator {}

        TargetTrace {
            visible: targetTraceModel.show
        }

        Component {
            id: measurementDelegate
            Measurement {
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: storedDelegate
            Stored {
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: unionDelegate
            Union {
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: standardLineDelegate
            StandardLine {
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: filterDelegate
            Filter {
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }
        Component {
            id: windowingDelegate
            Windowing {
                width: sideList.width
                dataModel: modelData
                highlight: modelHighlight
            }
        }

        Component {
            id: remoteItemDelegate
            RemoteItem {
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
            Layout.margins: 0
            spacing: 0
            reuseItems: false
            model: SourceModel {
                id: sourceModel
                list: sourceList
            }
            clip: true
            delegate: Component {
                id: delegateComponent

                MouseArea {
                    id: dragArea
                    property bool held: false
                    property int swipeStart: 0
                    property var source: model.source

                    anchors {
                        left: parent ? parent.left : delegateComponent.left
                        right: parent ? parent.right : delegateComponent.right
                    }
                    height: content.height

                    drag.target:    dragArea.held ? content : undefined
                    drag.axis:      Drag.YAxis
                    cursorShape:    Qt.PointingHandCursor
                    hoverEnabled: true
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

                        //swipe delete:

                        if ((swipeStart - mouseX) / content.width >= 1) {
                            if (applicationWindow && applicationWindow.properiesbar.currentObject === dragArea.source) {
                                applicationWindow.properiesbar.reset();
                            }
                            sourceList.removeItem(dragArea.source.data.uuid);
                        } else if ((swipeStart - mouseX) / content.width <= -0.5) {
                            content.opacity = 0;
                            content.height = 0;
                        } else {
                            content.opacity = 1;
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

                    onPressed: {
                        swipeStart = mouseX;
                    }

                    onPositionChanged: {
                        if (pressed) {
                            let opacity = 1 - Math.abs(mouseX - swipeStart) / content.width;
                            if (opacity > 0.6) opacity = 1.0;
                            content.opacity = opacity;
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
                            property var modelData: dragArea.source
                            property bool modelHighlight: index == sideList.currentIndex
                            sourceComponent: {
                                    switch(model.name) {
                                        case "Measurement": return measurementDelegate;
                                        case "Stored": return storedDelegate;
                                        case "Union": return unionDelegate;
                                        case "StandardLine": return standardLineDelegate;
                                        case "Filter": return filterDelegate;
                                        case "Windowing": return windowingDelegate;

                                        case "RemoteItem":
                                        case "RemoteStored":
                                        case "RemoteMeasurement":
                                            return remoteItemDelegate;
                                        default: console.log("unknow model " + model.name);return ;
                                    }
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
                            id: cloneButton
                            font.family: "Osm"
                            text: "\uf24d"
                            anchors.right: spacer.left
                            anchors.top: parent.top
                            flat: true
                            font.pixelSize: 14
                            rightPadding: 4
                            leftPadding: 4
                            visible: (dragArea.source.data ? dragArea.source.data.cloneable : false)
                            onClicked: {
                                sourceList.cloneItem(dragArea.source);
                            }
                            background: Rectangle {
                                color: "transparent"
                            }
                        }

                        Rectangle {
                            id: spacer
                            anchors {
                                top: parent.top
                                bottom: parent.bottom
                                right: deleteButton.left
                            }
                            width: 6
                            color: "transparent"
                        }

                        Button {
                            id: deleteButton
                            font.family: "Osm"
                            text: "\ue801"
                            anchors.right: parent.right
                            anchors.top: parent.top
                            flat: true
                            font.pixelSize: 14
                            rightPadding: 4
                            leftPadding: 4
                            onClicked: {
                                applicationWindow.dialog.accepted.connect(deleteModel);
                                applicationWindow.dialog.rejected.connect(freeDialog);
                                applicationWindow.dialog.title = "Delete " + dragArea.source.data.name + "?";
                                applicationWindow.dialog.open();
                            }
                            function deleteModel() {
                                if (applicationWindow.properiesbar.currentObject === dragArea.source) {
                                    applicationWindow.properiesbar.reset();
                                }
                                sourceList.removeItem(dragArea.source.data.uuid);
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

                        ToolTip {
                            text: "delete source"
                            visible: deleteButton.hovered
                            y: bottomPadding - deleteButton.height
                            x: content.width - rightPadding - availableWidth - leftMargin - rightMargin
                        }

                        ToolTip {
                            text: "clone source"
                            visible: cloneButton.hovered
                            y: bottomPadding - cloneButton.height
                            x: content.width - rightPadding - availableWidth - leftMargin - rightMargin
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
                border.color: sourceList.highlightColor
                border.width: 0.5
                visible: sourceList.selectedIndex >= 0 ? true : false
                color: "transparent"
            }

            Connections {
                target: sourceList
                function onSelectedChanged() {
                    sideList.currentIndex = sourceList.selectedIndex;
                }
            }

            Shortcut {
                sequence: "Ctrl+X"
                context: Qt.ApplicationShortcut
                onActivated: {
                    for (var i = 0; i < sourceList.count; i++) {
                        if (sourceList.get(i) && sourceList.get(i).active &&
                                (sourceList.get(i).objectName === "Measurement" ||
                                 sourceList.get(i).objectName === "Union")
                        ) {
                            var stored = sourceList.get(i).store();

                            stored.autoName(sourceList.get(i).name);
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

            Shortcut {
                sequence: "Ctrl+5"
                onActivated: {
                    sourceModel.layoutChanged();
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 8

            MouseArea {
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: aboutButton.height + aboutButton.topPadding + aboutButton.bottomPadding
                Layout.fillWidth: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    aboutpopup.open();
                }

                RowLayout {
                    spacing: 0
                    anchors.fill: parent
                    Layout.alignment: Qt.AlignCenter
                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        id: logoImage
                        source: "qrc:/images/icons/white80.png"
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 30
                        Layout.alignment: Qt.AlignCenter

                        RotationAnimation on rotation {
                            id: rotateImage
                            from: 0
                            to: 360
                            duration: 1200
                        }

                        Timer {
                            interval: 614657
                            running: true
                            repeat: true
                            onTriggered: rotateImage.start()
                        }
                    }

                    Label {
                        id:aboutButton
                        text: "ABOUT"
                        verticalAlignment: Text.AlignVCenter
                        rightPadding: 8
                        leftPadding: 8
                        bottomPadding: 8
                        topPadding: 8
                        font.pointSize: 11
                        Material.foreground: Material.Indigo
                        height: 48
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }

            Button {
                font.family: "Osm"
                text: "\ue807"
                flat: true

                PropertiesOpener {
                   propertiesQml: "qrc:/RemoteProperties.qml"
                   onClicked: {
                       open();
                   }
                }
            }

            Item {
                Layout.preferredWidth: 0.1
            }
        }
    }
}
