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
import QtQuick 2.7
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import SourceModel 1.0

ApplicationWindow {
    id:applicationWindow

    property alias properiesbar: bottomtab
    property alias charts: charts
    property alias dataSourceList : righttab
    property alias message : message
    property alias dialog : dialog
    property alias darkMode : darkModeSelect.checked
    //NOTE: Properties for fix Menu colors
    property string backgroundColor: Material.backgroundColor
    property string accentColor: Material.accent
    property string foregroundColor: Material.foreground

    visible: true
    flags: Qt.Window

    Component.onCompleted: {
        var mainwindowSettings = applicationSettings.getGroup("mainwindow");

        x       = mainwindowSettings.value("x", (Screen.width  - width)  / 2);
        y       = mainwindowSettings.value("y", (Screen.height - height) / 2);
        width   = mainwindowSettings.value("width", 1000);
        height  = mainwindowSettings.value("height", 600);

        applicationWindow.onWidthChanged.connect(function() {applicationSettings.setValue("mainwindow/width", width)});
        applicationWindow.onHeightChanged.connect(function() {applicationSettings.setValue("mainwindow/height", height)});
        applicationWindow.onXChanged.connect(function() {applicationSettings.setValue("mainwindow/x", x)});
        applicationWindow.onYChanged.connect(function() {applicationSettings.setValue("mainwindow/y", y)});
    }
    minimumWidth: 1000
    minimumHeight: 600
    color: darkModeSelect.checked ? Material.backgroundColor : "#FFFFFF"
    Material.theme: darkModeSelect.checked ? Material.Dark : Material.Light
    Material.accent: Material.Indigo

    menuBar: MenuBar {
        style: MenuBarStyle{
                    background: Rectangle{ color:Material.backgroundColor}
                    itemDelegate: Rectangle {
                        implicitWidth: menuBarLabel.contentWidth * 1.4
                        implicitHeight: menuBarLabel.contentHeight * 1.5
                        color: styleData.selected || styleData.open ? accentColor : backgroundColor
                        Label {
                            id:menuBarLabel
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: menuBarLabel.contentHeight/5
                            color: styleData.selected  || styleData.open ? backgroundColor : foregroundColor
                            text: formatMnemonic(styleData.text,true)
                        }
                    }
                    menuStyle: MenuStyle {
                        frame: Rectangle {
                            color: backgroundColor
                            border.width: 0
                        }

                        //FIXME: Colors doesn't set correctly from Material
                        itemDelegate {
                            background: Rectangle {
                                color:  styleData.selected || styleData.open ? accentColor : backgroundColor
                                border.width: 0
                            }
                            label: Label {
                                color: styleData.selected ? backgroundColor : foregroundColor
                                text: formatMnemonic(styleData.text,true)
                            }

                            submenuIndicator: Text {
                                text: "\u25ba"
                                color: styleData.selected  || styleData.open ? accentColor : backgroundColor
                            }

                            shortcut: Label {
                                color: styleData.selected ? backgroundColor : foregroundColor
                                text: styleData.shortcut
                            }

                            checkmarkIndicator: CheckBox {
                                checked: styleData.checked
                            }
                        }
                    }

                }

            Menu {
                title: qsTr("&File")
                MenuItem {
                    text: qsTr("&New")
                    shortcut: StandardKey.New
                    onTriggered: {
                        applicationWindow.properiesbar.clear();
                        sourceList.reset();
                    }
                }
                MenuItem {
                    text: qsTr("&Save")
                    shortcut: StandardKey.Save
                    onTriggered: saveDialog.open();
                }
                MenuItem {
                    text: qsTr("&Load")
                    shortcut: StandardKey.Open
                    onTriggered: openDialog.open()
                }
                MenuItem {
                    text: qsTr("&Append measurement")
                    shortcut: "Ctrl+A"
                    onTriggered: sourceList.addMeasurement();
                }
            }
            Menu {
                title: qsTr("&View")
                MenuItem {
                    id: darkModeSelect
                    text: qsTr("&Dark Mode")
                    shortcut: "Ctrl+D"
                    checkable: true
                    checked: applicationSettings.value("darkMode") === "true"
                    onCheckedChanged: {
                        applicationSettings.setValue("darkMode", darkModeSelect.checked)
                    }
                }
                MenuItem {
                    id: calculator
                    text: qsTr("&Calculator")
                    shortcut: "Ctrl+W"
                    checkable: false
                    onTriggered: {
                        applicationWindow.properiesbar.open(null, "qrc:/Calculator.qml");
                    }

                    onCheckedChanged: {
                        applicationSettings.setValue("darkMode", darkModeSelect.checked)
                    }
                }
            }

            Menu {
                title: qsTr("&Help")
                MenuItem {
                    text: qsTr("About")
                    onTriggered: aboutpopup.open();
                    shortcut: "Ctrl+H"
                }
                MenuItem {
                    text: qsTr("Check for update")
                    shortcut: "Ctrl+U"
                    onTriggered: update.show();
                }
            }
        }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            spacing: 0

            //Charts area
            Charts {
                id: charts
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            //Properties area
            PropetiesBar {
                id: bottomtab
                height: 120
                Layout.fillWidth: true
            }
        }

        SideBar {
            id: righttab
            Layout.fillHeight: true
            width: 200
        }
    }

    About {
        id: aboutpopup
        x: 100
        y: 100
        width: parent.width - 200
        height: parent.height - 200
    }

    Updater {
        id: update
    }

    Message {
        id: message
    }

    ModalDialog {
          id: dialog
      }

    FileDialog {
        id: saveDialog
        selectExisting: false
        title: qsTr("Please choose a file's name")
        folder: shortcuts.home
        defaultSuffix: "osm"
        nameFilters: ["Open Sound Meter (*.osm)"]
        onAccepted: sourceList.save(saveDialog.fileUrl);
    }

    FileDialog {
        id: openDialog
        selectExisting: true
        title: qsTr("Please choose a file's name")
        folder: shortcuts.home
        defaultSuffix: "osm"
        nameFilters: ["Open Sound Meter (*.osm)"]
        onAccepted: function() {
            applicationWindow.properiesbar.clear();
            if (!sourceList.load(openDialog.fileUrl)) {
                message.showError(qsTr("could not open the file"));
            }
        }
    }
}
