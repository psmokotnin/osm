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
import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Popup {
    id: popup
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    ColumnLayout {
        anchors.fill: parent
        Text {
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredWidth: popup.availableWidth
            Layout.preferredHeight: 150
            textFormat: Text.RichText
            text: qsTr("
                <h1>OSM</h1><br/>
                <a href=\"https://psmokotnin.github.io/osm/\">https://psmokotnin.github.io/osm/</a><br><br/>
                Copyright (C) 2018  Pavel Smokotnin<br/>
                Version: %1<br/>
                License: GPL v3.0
            ").arg(appVersion)
            onLinkActivated: Qt.openUrlExternally(link)
        }

        Text {
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.RichText
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: popup.availableWidth
            wrapMode: Text.Wrap
            text: qsTr(
                "This program comes with ABSOLUTELY NO WARRANTY. " +
                "This is free software, and you are welcome to redistribute it " +
                "under certain conditions.<br/>" +
                '<a href="https://raw.githubusercontent.com/psmokotnin/osm/master/LICENSE">Full text of license</a>'
            )
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
