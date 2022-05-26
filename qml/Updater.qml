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
    width: 300
    height: 100
    x: (parent ? (parent.width  - width) / 2 : 100)
    y: (parent ? (parent.height - height) / 2 : 100)

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Label {
            id: update;
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            onLinkActivated: Qt.openUrlExternally(link)

            text: qsTr("Checking for update...")
        }

    Component.onCompleted: {
        check (true);
    }

    function show() {
        update.text = qsTr("Checking for update...")
        open();
        check();
    }

    function check (openonavailable) {
        request('http://api.opensoundmeter.com/check/' + Qt.platform.os, function (o) {

            if (o.readyState !== 4) { return; }

            try {
                var JsonObject = JSON.parse(o.responseText);
                if (JsonObject.tag_name && JsonObject.tag_name !== appVersion) {

                    update.text = qsTr("
                        Your version (%3) is different then the latest release.<br/>
                        <br/>
                        Click <a href=\"%2\" style=\"color:%4\">here</a> to download %1.
                    ")
                        .arg(JsonObject.tag_name)
                        .arg(JsonObject.html_url)
                        .arg(appVersion)
                        .arg(applicationWindow.accentColor)
                    ;
                    if (openonavailable) {
                        popup.open();
                    }
                } else {
                    update.text = qsTr("You have the latest version :)<br/>%1").arg(appVersion);
                }
            } catch (e) {
                update.text = qsTr(
                            "Error happend while checking.<br />
                            Check your internet connection
                ");
            }
        });
    }

    function request(url, callback) {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = (function(myxhr) {
            return function() {
                callback(myxhr);
            }
        })(xhr);
        xhr.open('GET', url, true);
        xhr.send('');
    }
}
