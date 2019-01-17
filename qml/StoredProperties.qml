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
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {

    property var dataObject

    RowLayout {
        spacing: 0

        TextField {
            placeholderText: qsTr("title")
            text: dataObject.name
            onTextEdited: dataObject.name = text

        }

        ColorPicker {
            id: colorPicker

            Layout.preferredWidth: 25
            Layout.preferredHeight: 25
            Layout.margins: 5

            onColorChanged: {
                dataObject.color = color
            }
        }

        Button {
            text: qsTr("Delete");
            onClicked: {
                applicationWindow.dataSourceList.remove(dataObject);
                applicationWindow.properiesbar.clear();
            }
        }

        Component.onCompleted: {
            colorPicker.color = dataObject.color
        }
    }
}
