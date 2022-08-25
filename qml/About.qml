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
import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1

Popup {
    id: popup
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    Item {
        anchors.fill: parent
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10

        ColumnLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 20

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: false

                Image {
                    source: "qrc:/images/icons/whitec.png"
                    Layout.preferredHeight: 100
                    Layout.preferredWidth: 100
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }

                Item {
                    Layout.fillWidth: true
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.fillHeight: false
                    Layout.fillWidth: false

                    Label {
                        horizontalAlignment: Text.AlignHCenter
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        Layout.fillWidth: true
                        font.pointSize: 20
                        text: qsTr("Open Sound Meter %1").arg(appVersion)
                    }

                    Label {
                        horizontalAlignment: Text.AlignHCenter
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        Layout.fillWidth: true
                        textFormat: Text.RichText
                        text: qsTr(
                                "<a style='color:%1' href=\"https://opensoundmeter.com\">https://opensoundmeter.com</a><br/><br/>" +
                                "Copyright 2017 — %2  Pavel Smokotnin. <br/>" +
                                "License: " +
                                '<a style="color:%1" href="https://raw.githubusercontent.com/psmokotnin/osm/master/LICENSE">GPL v3.0</a>'
                            ).arg(Material.accentColor).arg(new Date().getFullYear())
                        onLinkActivated: Qt.openUrlExternally(link)
                     }
                }
            }

            Label {
                text: qsTr("Distributed under <b>pay what you want</b> model")
                Layout.fillHeight: false
                Layout.fillWidth: false
                Layout.columnSpan: 2
                onLinkActivated: Qt.openUrlExternally(link)
            }

            RowLayout {
                Label {
                    text: qsTr(
                               "<a style='color:%1' href=\"https://opensoundmeter.com/about#donate\">donate</a><br/><br/>" +
                               "<a style='color:%1' href=\"https://opensoundmeter.com/support\">user manual</a><br/>" +
                               "<a style='color:%1' href=\"https://opensoundmeter.com/consulting\">consulting</a><br/>" +
                               "<a style='color:%1' href=\"https://opensoundmeter.com/training\">trainings</a><br/>" +
                               "<a style='color:%1' href=\"https://www.facebook.com/opensoundmeter/\">community</a><br/>"
                              ).arg(Material.accentColor)
                    textFormat: Text.RichText
                    horizontalAlignment: Text.AlignLeft
                    Layout.preferredWidth: 200
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    onLinkActivated: Qt.openUrlExternally(link)
                }

                MouseArea {
                    id: mouseArea1
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.fillWidth: false
                    Layout.preferredWidth: parent.width / 2
                    Layout.preferredHeight: 100

                    cursorShape: Qt.PointingHandCursor
                    onClicked:  {
                        Qt.openUrlExternally("https://opensoundmeter.com/qr");
                    }

                    Image {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        source: "qrc:/images/icons/qrCode.png"
                        anchors.rightMargin: 0
                        anchors.bottomMargin: 0
                        anchors.topMargin: 0
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredHeight: 100
                        Layout.preferredWidth: 100
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                }
            }

            Label {
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignBottom
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: 20
            }

            MouseArea {
                id: mouseArea
                Layout.fillWidth: true
                cursorShape: Qt.PointingHandCursor
                onClicked:  {
                    Qt.openUrlExternally("https://apps.apple.com/app/apple-store/id1552933259?pt=122681649&ct=inapp&mt=8");
            }

            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            RowLayout {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                Label {
                    id: foriOs
                    text: qsTr("Open Sound Meter for iOS:")
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

    MouseArea {
        anchors.right: parent.right
        anchors.top: parent.top
        cursorShape: Qt.PointingHandCursor
        implicitWidth: closeLabel.implicitWidth
        implicitHeight: closeLabel.implicitHeight

        Label {
            id: closeLabel
            font.family: "Osm"
            font.pixelSize: 20
            text: "\ue803"
        }
        onClicked: {
            popup.close();
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;height:400;width:568}
}
##^##*/
