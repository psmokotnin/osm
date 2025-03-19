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
import OpenSoundMeterModule 1.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

MenuBar {
        Menu {
            title: qsTr("&File")
            Action {
                text: qsTr("&New")
                shortcut: StandardKey.New
                onTriggered: {
                    dialog.title = qsTr("Create new workspace?")
                    dialog.accepted.connect(closeAccepted)
                    dialog.rejected.connect(closeRejected)
                    dialog.open()
                }
                function closeAccepted() {
                    applicationWindow.propertiesBar.clear()
                    sourceList.reset()
                    closeRejected()
                }
                function closeRejected() {
                    dialog.accepted.disconnect(closeAccepted)
                    dialog.rejected.disconnect(closeRejected)
                }
            }
            Action {
                text: qsTr("&Save")
                shortcut: StandardKey.Save
                onTriggered: saveDialog.open()
            }
            Action {
                text: qsTr("&Open")
                shortcut: StandardKey.Open
                onTriggered: openDialog.open()
            }
            Menu {
                title: qsTr("Recent projects")
                id: recentFilesMenu

                Instantiator {
                    model: recentFilesModel
                    onObjectAdded: recentFilesMenu.insertItem(index, object)
                    onObjectRemoved: recentFilesMenu.removeItem(object)
                }

                MenuSeparator {
                    visible: recentFilesModel.count > 0
                }

                Action {
                    text: qsTr("Clear menu")
                    enabled: recentFilesModel.count > 0
                    onTriggered: recentFilesModel.clear()
                }
            }
            Action {
                text: qsTr("&Import")
                shortcut: "Ctrl+I"
                onTriggered: importDialog.open()
            }
            Action {
                text: qsTr("&Add measurement")
                shortcut: "Ctrl+A"
                onTriggered: sourceList.addMeasurement()
            }
            Action {
                text: qsTr("&Add math source")
                shortcut: "Ctrl+M"
                onTriggered: sourceList.addUnion()
            }
            Action {
                text: qsTr("&Add standard line")
                shortcut: "Ctrl+L"
                onTriggered: sourceList.addStandardLine()
            }
            Action {
                text: qsTr("&Add filter")
                shortcut: "Ctrl+F"
                onTriggered: sourceList.addFilter()
            }
            Action {
                text: qsTr("&Add windowing")
                shortcut: "Ctrl+W"
                onTriggered: sourceList.addWindowing()
            }
            Action {
                text: qsTr("&Add Group")
                shortcut: "Ctrl+0"
                onTriggered: sourceList.addGroup()
            }
            Action {
                text: qsTr("&Show target")
                shortcut: "Ctrl+T"
                checkable: true
                checked: targetTraceModel.show
                onToggled: targetTraceModel.show = checked
            }
            Action {
                text: qsTr("Quit")
                shortcut: StandardKey.Quit
                onTriggered: applicationWindow.close()
            }
        }
        Menu {
            title: qsTr("&View")
            Action {
                text: qsTr("&Dark Mode")
                shortcut: "Ctrl+D"
                checkable: true
                checked: applicationAppearance.darkMode
                onToggled: applicationAppearance.darkMode = checked
            }
            Action {
                text: qsTr("&Calculator")
                shortcut: "Ctrl+K"
                onTriggered: applicationWindow.propertiesBar.open(null, "qrc:/Calculator.qml")
            }
            Action {
                text: qsTr("&Experiment functions")
                checkable: true
                checked: applicationAppearance.experimentFunctions
                onToggled: applicationAppearance.experimentFunctions = checked
            }
        }
        Menu {
            title: qsTr("&Help")
            Action {
                text: qsTr("&Shortcuts")
                shortcut: "F1"
                onTriggered: shortcutsPopup.open()
            }
            Action {
                text: qsTr("About")
                shortcut: "F2"
                onTriggered: aboutPopup.open()
            }
            Action {
                text: qsTr("Check for update")
                shortcut: "F3"
                onTriggered: update.show()
            }
        }
    }
