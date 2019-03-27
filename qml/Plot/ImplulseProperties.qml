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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3

import "../" as Root

Item {
    id: chartProperties
    property var dataObject

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

    RowLayout {
        spacing: 0

        Root.FloatSpinBox {
            min: dataObject.xLowLimit
            max: dataObject.xHighLimit
            value: dataObject.xmin
            tooltiptext: qsTr("x from")
            onValueChanged: dataObject.xmin = value
            implicitWidth: 170
            Layout.fillWidth: true
        }

        Root.FloatSpinBox {
            min: dataObject.xLowLimit
            max: dataObject.xHighLimit
            value: dataObject.xmax
            tooltiptext: qsTr("x to")
            onValueChanged: dataObject.xmax = value
            implicitWidth: 170
            Layout.fillWidth: true
        }

        Root.FloatSpinBox {
            min: dataObject.yLowLimit
            max: dataObject.yHighLimit
            value: dataObject.ymin
            tooltiptext: qsTr("y from")
            onValueChanged: dataObject.ymin = value
            implicitWidth: 170
            Layout.fillWidth: true
        }

        Root.FloatSpinBox {
            min: dataObject.yLowLimit
            max: dataObject.yHighLimit
            value: dataObject.ymax
            tooltiptext: qsTr("y to")
            onValueChanged: dataObject.ymax = value
            implicitWidth: 170
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Save Image");
            implicitWidth: 120
            onClicked: fileDialog.open();
        }
    }
    RowLayout {
        spacing: 0


        FileDialog {
            id: fileDialog
            selectExisting: false
            title: "Please choose a file's name"
            folder: shortcuts.home
            defaultSuffix: "png"
            onAccepted: {
                dataObject.parent.grabToImage(function(result) {
                    result.saveToFile(dataObject.parent.urlForGrab(fileDialog.fileUrl));
                });
            }
        }
    }
  }
}
