/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

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

    ColumnLayout {
        anchors.fill: parent
        spacing: 20
        anchors.bottomMargin: 20
        anchors.topMargin: 20
        anchors.rightMargin: 0
        anchors.leftMargin: 0

        RowLayout {
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            Image {
                source: "qrc:/images/icons/whitec.png"
                Layout.preferredHeight: 100
                Layout.preferredWidth: 100
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            }

            ColumnLayout {
                Layout.fillHeight: false
                Layout.fillWidth: false
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    font.pointSize: 20
                    text: qsTr("Open Sound Meter %1").arg(appVersion)
                }

                Label {
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    textFormat: Text.RichText
                    text: qsTr(
                            "<a style='color:%1' href=\"https://opensoundmeter.com\">https://opensoundmeter.com</a><br/><br/>" +
                            "Copyright (C) %2  Pavel Smokotnin<br/>" +
                            "License: " +
                            '<a style="color:%1" href="https://raw.githubusercontent.com/psmokotnin/osm/master/LICENSE">GPL v3.0</a>'
                        ).arg(Material.accentColor).arg(new Date().getFullYear())
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            onClicked:  {
                Qt.openUrlExternally("https://apps.apple.com/app/id1552933259");
            }

            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Image {
                id: appstore
                source: "qrc:/images/icons/appstore.png"
                fillMode: Image.PreserveAspectFit
                width: 120
                height: 40
                anchors.centerIn: parent
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true
            Label {
                text: qsTr("Like desktop version? Please <b>pay what you want</b> — to help me make further improvements and updates.<br/><br/>" +
                           "Write your ideas and questions in comments on " +
                           "<a style='color:%1' href=\"https://www.facebook.com/opensoundmeter/\">facebook</a>!"
                           ).arg(Material.accentColor)
                Layout.fillHeight: false
                Layout.fillWidth: false
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        RowLayout {
            Layout.fillHeight: true
            Layout.rightMargin: 40
            Layout.leftMargin: 40
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                onClicked:  {
                    Qt.openUrlExternally("https://pay.cloudtips.ru/p/28bedd0a");
                }

                Layout.fillWidth: true
                Layout.preferredHeight: 100

                ColumnLayout {
                    spacing: 5
                    anchors.fill: parent
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    Label {
                        text: "Donate with credit card"
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        Layout.topMargin: 0
                        color: Material.color(Material.Indigo)
                    }

                    Image {
                        id: qrcode
                        source: "qrc:/images/icons/qrCode.png"
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredHeight: 100
                        Layout.preferredWidth: 100
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }

                    Label {
                        text: "click or scan QR with your phone"
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        Layout.topMargin: 0
                        color: Material.color(Material.Grey)
                    }
                }
            }

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                onClicked:  {
                    Qt.openUrlExternally("https://www.paypal.com/paypalme/psmokotnin");
                }

                ColumnLayout {
                    spacing: 5
                    anchors.fill: parent

                    Label {
                        text: "Donate via PayPal"
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        Layout.topMargin: 0
                        color: Material.color(Material.Indigo)
                    }

                    Image {
                        source: "qrc:/images/icons/pp.jpg"
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredHeight: 100
                        Layout.preferredWidth: 100
                        Layout.alignment: Qt.AlignCenter
                    }

                    Label {
                        text: "paypal.me/psmokotnin"
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        Layout.topMargin: 0
                        color: Material.color(Material.Grey)
                    }
                }
            }
        }

        Label {
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignBottom
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.preferredHeight: 25
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:480;width:1240}
}
##^##*/
