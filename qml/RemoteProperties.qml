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

Item {

    GridLayout {
        anchors.fill: parent
        columns: 3

        Label {
            text: "Server:"
        }

        Button {
            checkable: true
            text: qsTr("Active")
            checked: remoteServer.active
            onCheckedChanged: {
                remoteServer.active = checked;
            }
        }

        Item {
            Layout.fillWidth: true
        }

        Label {
            opacity: typeof(remoteClient) != "undefined"

            text: "Client:"
        }

        Button {
            opacity: typeof(remoteClient) != "undefined"

            checkable: true
            text: qsTr("Active")
            checked: remoteClient.active
            onCheckedChanged: {
                remoteClient.active = checked;
            }
        }


        MouseArea {
            id: mouseArea
            Layout.fillWidth: true
            opacity: typeof(remoteClient) == "undefined"
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            cursorShape: Qt.PointingHandCursor
            onClicked:  {
                Qt.openUrlExternally("https://apps.apple.com/app/id1552933259");
            }

            RowLayout {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                Label {
                    id: foriOs
                    text: qsTr("Use Open Sound Meter for iOS as a client:")
                    horizontalAlignment: Text.AlignRight
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    onLinkActivated: Qt.openUrlExternally(link)
                }

                Image {
                    id: appstore
                    source: "qrc:/images/icons/appstore.png"
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 40
                }
            }
        }
    }
}
