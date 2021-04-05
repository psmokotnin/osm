/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQml.Models 2.15

Drawer {
    id: drawer
    property bool darkMode: false

    width: righttab.width
    height: applicationWindow.height

    interactive: true
    edge: Qt.RightEdge

    ListModel {
        id: actions
        ListElement {
            name: qsTr("New")
            onclick: function() {
                applicationWindow.properiesbar.clear();
                sourceList.reset();
            }
        }
        ListElement {
            name: qsTr("Save")
            onclick: function() {saveDialog.open();}
        }
        ListElement {
            name: qsTr("Open")
            onclick: function() {openDialog.open();}
        }
        ListElement {
            name: qsTr("Import")
            onclick: function() {importDialog.open();}
            separator: true
        }

        ListElement {
            name: qsTr("Add measurement")
            onclick: function() {sourceList.addMeasurement();}
        }
        ListElement {
            name: qsTr("Add math source")
            onclick: function() {sourceList.addUnion();}
        }
        ListElement {
            name: qsTr("Add elc")
            onclick: function() {sourceList.addElc();}
            separator: true
        }

        ListElement {
            name: qsTr("Calculator")
            onclick: function() {
                applicationWindow.properiesbar.open(null, "qrc:/Calculator.qml");
            }
        }
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        Image {
            source: "qrc:/images/icons/white80.png"
            Layout.preferredHeight: 50
            Layout.preferredWidth: 50
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 20;
        }

        Repeater {
            delegate: ColumnLayout {
                Layout.fillWidth: true
                Button {
                    text: name
                    onClicked: {
                        onclick();
                        drawer.close();
                    }
                    flat: true
                    Layout.fillWidth: true
                }
                ToolSeparator {
                    visible: separator
                    orientation: Qt.Horizontal
                    Layout.fillWidth: true
                }
            }
            model: actions
        }
    }
}
