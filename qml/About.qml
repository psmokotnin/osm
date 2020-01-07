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

        Label {
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            font.pointSize: 20
            text: "Open Sound Meter"
        }

        Image {
            source: "qrc:/images/icons/white.png"
            Layout.preferredHeight: 100
            Layout.preferredWidth: 100
            Layout.alignment: Qt.AlignCenter
        }

        Label {
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            Layout.bottomMargin: 50
            textFormat: Text.RichText
            text: qsTr("
                    <a style='color:%2' href=\"https://psmokotnin.github.io/osm/\">https://psmokotnin.github.io/osm/</a><br><br/>
                    Copyright (C) 2020  Pavel Smokotnin<br/>
                    Version: %1<br/>
                    License: GPL v3.0
                ").arg(appVersion).arg(Material.accentColor)
            onLinkActivated: Qt.openUrlExternally(link)
        }

        Label {
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.RichText
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 80
            Layout.leftMargin: 100
            Layout.rightMargin: 100
            wrapMode: Text.Wrap
            text: qsTr(
                      "This program comes with ABSOLUTELY NO WARRANTY. " +
                      "This is free software, and you are welcome to redistribute it " +
                      "under certain conditions.<br/>" +
                      '<a style="color:%1" href="https://raw.githubusercontent.com/psmokotnin/osm/master/LICENSE">Full text of license</a>'
                      ).arg(Material.accentColor)
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
