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
    property var dataObject

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {

            Button {
                text: qsTr("Refresh")
                onClicked: dataObject.data.refresh();
            }

            Label {
                text: dataObject.data ? dataObject.data.name : ""
            }

            Label {
                text: "@"
            }

            Label {
                text: dataObject.data ? dataObject.data.host : ""
            }
        }

        RowLayout {
        }
    }
}
