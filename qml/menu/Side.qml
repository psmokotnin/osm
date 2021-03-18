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
                    Layout.alignment: Qt.AlignLeft//!!!!!!!!!!!!!!!!!!FIX!!!
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

    ScrollIndicator.vertical: ScrollIndicator { }
/*

    Menu {
        title: qsTr("Help")
        MenuItem {
            text: qsTr("Shortcuts")
            shortcut: "F1"
            checkable: false
            onTriggered: shortcutsPopup.open();
        }
        MenuItem {
            text: qsTr("About")
            onTriggered: aboutpopup.open();
            shortcut: "F2"
        }
        MenuItem {
            text: qsTr("Check for update")
            shortcut: "F3"
            onTriggered: update.show();
        }
    }
*/
}
