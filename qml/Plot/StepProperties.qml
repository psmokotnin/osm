/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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

import "../elements"
import SourceModel 1.0
import OpenSoundMeter 1.0

Item {
    id: chartProperties
    property var dataObject

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

    RowLayout {
        spacing: 0

        FloatSpinBox {
            id: xminFloatBox
            min: dataObject.xLowLimit
            max: dataObject.xHighLimit
            value: dataObject.xmin
            tooltiptext: qsTr("x from")
            onValueChanged: dataObject.xmin = value
            implicitWidth: 170
            Layout.fillWidth: true
            units: "ms"
        }

        FloatSpinBox {
            id: xmaxFloatBox
            min: dataObject.xLowLimit
            max: dataObject.xHighLimit
            value: dataObject.xmax
            tooltiptext: qsTr("x to")
            onValueChanged: dataObject.xmax = value
            implicitWidth: 170
            Layout.fillWidth: true
            units: "ms"
        }

        FloatSpinBox {
            id: yminFloatBox
            min: dataObject.yLowLimit
            max: dataObject.yHighLimit
            value: dataObject.ymin
            tooltiptext: qsTr("y from")
            onValueChanged: dataObject.ymin = value
            implicitWidth: 170
            Layout.fillWidth: true
        }

        FloatSpinBox {
            id: ymaxFloatBox
            min: dataObject.yLowLimit
            max: dataObject.yHighLimit
            value: dataObject.ymax
            tooltiptext: qsTr("y to")
            onValueChanged: dataObject.ymax = value
            implicitWidth: 170
            Layout.fillWidth: true
        }

        Connections {
            target: dataObject
            function onXminChanged() {
                xminFloatBox.value = dataObject.xmin;
            }
            function onXmaxChanged() {
                xmaxFloatBox.value = dataObject.xmax;
            }
            function onYminChanged() {
                yminFloatBox.value = dataObject.ymin;
            }
            function onYmaxChanged() {
                ymaxFloatBox.value = dataObject.ymax;
            }
        }

        Button {
            font.family: "Osm"
            text: "\ue804"
            implicitWidth: 60
            onClicked: fileDialog.open();
            ToolTip.visible: hovered
            ToolTip.text: qsTr("save chart as an image")
        }
    }
    RowLayout {
        spacing: 0

        FloatSpinBox {
            id: zeroPoinntFloatSpinBox
            min: dataObject.xLowLimit
            max: dataObject.xHighLimit
            value: dataObject.zero
            units: "ms"
            tooltiptext: qsTr("integration zero point")
            onValueChanged: dataObject.zero = value
            Layout.preferredWidth: xminFloatBox.width
        }

        RowLayout {
            Layout.fillWidth: true
        }

        Select {
            id: selectFilter
            tooltip: qsTr("show only selected sources")
            sources: sourceList
            dataObject: chartProperties.dataObject
            Layout.preferredWidth: 200
        }


        FileDialog {
            id: fileDialog
            selectExisting: false
            title: "Please choose a file's name"
            folder: (typeof shortcuts !== 'undefined' ? shortcuts.home : Filesystem.StandardFolder.Home)
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
