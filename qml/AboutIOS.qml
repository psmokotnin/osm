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
            Layout.alignment: Qt.AlignHCenter | Qt.AlignCenter

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
                            "Copyright (C) %2  Pavel Smokotnin"
                        ).arg(Material.accentColor).arg(new Date().getFullYear())
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:480;width:1240}
}
##^##*/
