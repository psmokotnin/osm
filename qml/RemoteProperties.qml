/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.13
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

Item {

    ColumnLayout {
        anchors.fill: parent

        RowLayout {

            Button {
                checkable: true
                text: qsTr("Server")
                checked: remoteServer.active
                Material.background: parent.Material.background
                onCheckedChanged: {
                    remoteServer.active = checked;
                }
            }

            Label {
                Layout.fillWidth: true
                text: remoteServer.lastConnected ? "Last connected client: <b>" + remoteServer.lastConnected + "</b>" : ""
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                textFormat: Text.RichText
            }

            Button {
                text: qsTr("Available clients")
                Layout.preferredWidth: 200
                Material.background: parent.Material.background

                onClicked:  {
                    Qt.openUrlExternally("https://opensoundmeter.com/api/clients");
                }
            }
        }

        RowLayout {
            Button {
                checkable: true
                enabled: remoteClient.licensed
                text: qsTr("Client")
                Material.background: parent.Material.background
                checked: remoteClient.active
                onCheckedChanged: {
                    remoteClient.active = checked;
                }
            }

            Label {
                Layout.fillWidth: true
                text: remoteClient.licensed ? "Licensed to <b>" + remoteClient.licenseOwner + "</b>" : "Key not found"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                textFormat: Text.RichText
            }

            Button {
                text: qsTr("Open")
                Layout.preferredWidth: 98
                Material.background: parent.Material.background
                onClicked: openLicenseDialog.open();
                enabled: !remoteClient.staticLicense
                visible: !remoteClient.staticLicense
            }

            Button {
                text: qsTr("Buy")
                Layout.preferredWidth: 97
                Material.background: parent.Material.background
                enabled: !remoteClient.staticLicense
                visible: !remoteClient.staticLicense
            }
        }
    }

    FileDialog {
        id: openLicenseDialog
        selectExisting: true
        title: qsTr("Please choose a license file")
        folder: (typeof shortcuts !== 'undefined' ? shortcuts.home : Filesystem.StandardFolder.Home)
        defaultSuffix: "osmkey"
        nameFilters: [
            "Open Sound Meter API license (*.osmkey)"
        ]
        onAccepted: function() {
            if (!remoteClient.openLicenseFile(openLicenseDialog.fileUrl)) {
                message.showError(qsTr("could not open license file"));
            }
        }
    }
}
