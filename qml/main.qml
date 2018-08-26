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
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

ApplicationWindow {
    id:applicationWindow

    property alias properiesbar: bottomtab
    property alias charts: charts
    property alias dataSourceList : righttab

    visible: true

    width: 1000
    height: 600
    minimumWidth: 1000
    minimumHeight: 600

    Material.theme: Material.Light
    Material.accent: Material.Indigo

    menuBar: MenuBar {
            Menu {
                title: qsTr("&Help")
                MenuItem {
                    text: qsTr("About")
                    onTriggered: aboutpopup.open();
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
}
