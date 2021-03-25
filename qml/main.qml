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
import QtQuick.Dialogs 1.2
import SourceModel 1.0
import OpenSoundMeter 1.0

import "menu" as Menu

ApplicationWindow {
    id:applicationWindow

    property alias properiesbar: bottomtab
    property alias charts: charts
    property alias dataSourceList : righttab
    property alias message : message
    property alias dialog : dialog
    property alias sideMenu: sideMenu
    property string backgroundColor: Material.backgroundColor
    property string accentColor: Material.accent
    property string foregroundColor: Material.foreground

    visible: true
    flags: Qt.Window

    Component.onCompleted: {
        if (applicationAppearance.visibility === Appearance.FullScreen) {
            return ;
        }

        var mainwindowSettings = applicationSettings.getGroup("mainwindow");

        x       = mainwindowSettings.value("x", (Screen.width  - width)  / 2);
        y       = mainwindowSettings.value("y", (Screen.height - height) / 2);
        width   = mainwindowSettings.value("width", minimumWidth);
        height  = mainwindowSettings.value("height", minimumHeight);

        applicationWindow.onWidthChanged.connect(function() {applicationSettings.setValue("mainwindow/width", width)});
        applicationWindow.onHeightChanged.connect(function() {applicationSettings.setValue("mainwindow/height", height)});
        applicationWindow.onXChanged.connect(function() {applicationSettings.setValue("mainwindow/x", x)});
        applicationWindow.onYChanged.connect(function() {applicationSettings.setValue("mainwindow/y", y)});
    }
    minimumWidth: 1080
    minimumHeight: 600
    visibility: applicationAppearance.visibility
    color: applicationAppearance.darkMode ? Material.backgroundColor : "#FFFFFF"
    Material.theme: applicationAppearance.darkMode ? Material.Dark : Material.Light
    Material.accent: Material.Indigo

    Menu.Side {
        id:sideMenu
        interactive: applicationAppearance.showMenuBar ? false : true
    }
    Menu.Top {
        id: topMenu;
    }

    menuBar: (applicationAppearance.showMenuBar ? topMenu : null)



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
        x: 50
        y: 50
        width: parent.width - 100
        height: parent.height - 100
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

    Shortcuts {
        id:shortcutsPopup
        anchors.centerIn: parent
    }

    FileDialog {
        id: saveDialog
        selectExisting: false
        title: qsTr("Please choose a file's name")
        folder: shortcuts.home
        defaultSuffix: "osm"
        nameFilters: ["Open Sound Meter (*.osm)"]
        onAccepted: sourceList.save(saveDialog.fileUrl);
        Component.onCompleted: {
            console.log(parent)
        }
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

    FileDialog {
        id: importDialog
        selectExisting: true
        title: qsTr("Please choose a file's name")
        folder: shortcuts.home
        defaultSuffix: "txt"
        nameFilters: ["txt transfer data file (*.txt)", "csv transfer data file (*.csv)"]
        onAccepted: function() {
            applicationWindow.properiesbar.clear();
            switch (nameFilters.indexOf(selectedNameFilter)) {
            case 0:
                if (!sourceList.importTxt(importDialog.fileUrl)) {
                    message.showError(qsTr("could not open the file"));
                }
                break;
            case 1:
                if (!sourceList.importCsv(importDialog.fileUrl)) {
                    message.showError(qsTr("could not open the file"));
                }
                break;
            }
        }
    }
}
