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
import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQml.Models 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import OpenSoundMeterModule 1.0

MenuBar {
    id: menuBar;
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
                dialog.title = qsTr("Create new workspace?")
                dialog.accepted.connect(closeAccepted);
                dialog.rejected.connect(closeRejected);
                dialog.open();
            }
            function closeAccepted() {
                applicationWindow.properiesbar.clear();
                sourceList.reset();
                closeRejected();
            }
            function closeRejected() {
                applicationWindow.dialog.accepted.disconnect(closeAccepted);
                applicationWindow.dialog.rejected.disconnect(closeRejected);
            }
        }
        MenuItem {
            text: qsTr("&Save")
            shortcut: StandardKey.Save
            onTriggered: saveDialog.open();
        }
        MenuItem {
            text: qsTr("&Open")
            shortcut: StandardKey.Open
            onTriggered: openDialog.open()
        }

        Menu {
            title: qsTr("Recent projects")
            id: recentFilesMenu

            RecentFilesModel {
                id: recentFilesModel
                settings: applicationSettings.getGroup("recentFiles");
                onProjectFolderChanged: {
                    let folder = recentFilesModel.projectFolder();
                    if (folder) {
                        openDialog.folder = folder;
                        saveDialog.folder = folder;
                    }
                }
            }

            Connections {
                target: sourceList
                function onLoaded(url) {
                    recentFilesModel.addUrl(url);
                }
            }

            Connections {
                target: saveDialog
                function onAccepted() {
                    recentFilesModel.storeProjectFolder(saveDialog.fileUrl);
                }
            }

            Connections {
                target: openDialog
                function onAccepted() {
                    recentFilesModel.storeProjectFolder(openDialog.fileUrl);
                }
            }

            Instantiator {
                model: recentFilesModel
                MenuItem {
                    text: model.fileName
                    onTriggered: sourceList.load(model.url)
                }
                onObjectAdded: recentFilesMenu.insertItem(index, object);
                onObjectRemoved: recentFilesMenu.removeItem(object)
            }

            MenuSeparator {
                visible: recentFilesModel.count > 0
            }

            MenuItem {
                text: "Clear menu"
                enabled: recentFilesModel.count > 0
                onTriggered: recentFilesModel.clear()
            }
        }
        MenuItem {
            text: qsTr("&Import")
            shortcut: "Ctrl+I"
            onTriggered: importDialog.open()
        }
        MenuItem {
            text: qsTr("&Add measurement")
            shortcut: "Ctrl+A"
            onTriggered: sourceList.addMeasurement();
        }
        MenuItem {
            text: qsTr("&Add math source")
            shortcut: "Ctrl+M"
            onTriggered: sourceList.addUnion();
        }
        MenuItem {
            text: qsTr("&Add standard line")
            shortcut: "Ctrl+L"
            onTriggered: sourceList.addStandardLine();
        }
        MenuItem {
            text: qsTr("&Add filter")
            shortcut: "Ctrl+F"
            onTriggered: sourceList.addFilter();
        }
        MenuItem {
            text: qsTr("&Add windowing")
            shortcut: "Ctrl+W"
            onTriggered: sourceList.addWindowing();
        }
        MenuItem {
            text: qsTr("&Add Group")
            shortcut: "Ctrl+0"
            onTriggered: sourceList.addGroup();
        }
        MenuItem {
            text: qsTr("&Show target")
            shortcut: "Ctrl+T"
            checkable: true
            checked: targetTraceModel.show
            onCheckedChanged: targetTraceModel.show = checked
        }
        MenuItem {
            text: qsTr("Quit")
            shortcut: StandardKey.Quit
            onTriggered: applicationWindow.close();
        }
    }
    Menu {
        title: qsTr("&View")
        MenuItem {
            id: darkModeSelect
            text: qsTr("&Dark Mode")
            shortcut: "Ctrl+D"
            checkable: true
            checked: applicationAppearance.darkMode
            onCheckedChanged: {
                applicationAppearance.darkMode = darkModeSelect.checked;
            }
        }
        MenuItem {
            id: calculator
            text: qsTr("&Calculator")
            shortcut: "Ctrl+K"
            checkable: false
            onTriggered: {
                applicationWindow.properiesbar.open(null, "qrc:/Calculator.qml");
            }
        }
        MenuItem {
            id: experimentFunctions
            text: qsTr("&Experiment functions")
            checkable: true
            checked: applicationAppearance.experimentFunctions
            onCheckedChanged: {
                applicationAppearance.experimentFunctions = experimentFunctions.checked;
            }
        }
    }

    Menu {
        title: qsTr("&Help")
        MenuItem {
            text: qsTr("&Shortcuts")
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
}
