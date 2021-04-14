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
import SourceModel 1.0
import OpenSoundMeter 1.0

Item {
    id: chartProperties
    property var dataObject

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

    RowLayout {
        spacing: 0

        SpinBox {
            value: dataObject.xmin
            onValueChanged: dataObject.xmin = value
            from: dataObject.xLowLimit
            to: dataObject.xHighLimit
            editable: true
            implicitWidth: 170
            Layout.fillWidth: true

            ToolTip.visible: hovered
            ToolTip.text: qsTr("x from")

            textFromValue: function(value, locale) {
                return Number(value) + "Hz"
            }

            valueFromText: function(text, locale) {
                return parseInt(text)
            }
        }

        SpinBox {
            value: dataObject.xmax
            onValueChanged: dataObject.xmax = value
            from: dataObject.xLowLimit
            to: dataObject.xHighLimit
            editable: true
            implicitWidth: 170
            Layout.fillWidth: true

            ToolTip.visible: hovered
            ToolTip.text: qsTr("x to")

            textFromValue: function(value, locale) {
                return Number(value) + "Hz"
            }

            valueFromText: function(text, locale) {
                return parseInt(text)
            }
        }

        SpinBox {
            value: dataObject.ymin
            onValueChanged: dataObject.ymin = value
            from: dataObject.yLowLimit
            to: dataObject.yHighLimit
            editable: true
            implicitWidth: 170
            Layout.fillWidth: true

            ToolTip.visible: hovered
            ToolTip.text: qsTr("y from")

            textFromValue: function(value, locale) {
                return Number(value) + "dB"
            }

            valueFromText: function(text, locale) {
                return parseInt(text)
            }
        }

        SpinBox {
            value: dataObject.ymax
            onValueChanged: dataObject.ymax = value
            from: dataObject.yLowLimit
            to: dataObject.yHighLimit
            editable: true
            implicitWidth: 170
            Layout.fillWidth: true

            ToolTip.visible: hovered
            ToolTip.text: qsTr("y to")

            textFromValue: function(value, locale) {
                return Number(value) + "dB"
            }

            valueFromText: function(text, locale) {
                return parseInt(text)
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

        Root.TitledCombo {
            id: mode
            implicitWidth: 170
            title: qsTr("mode")
            tooltip: qsTr("render data as")
            model: ["line", "bars", "lines"]
            currentIndex: dataObject.mode;
            onCurrentIndexChanged: {dataObject.mode = currentIndex;}
        }

        Root.TitledCombo {
            title: qsTr("ppo")
            tooltip: qsTr("points per octave")
            implicitWidth: 170
            visible: mode.model[mode.currentIndex] === "bars"
            model: [3, 6, 12, 24, 48]
            Component.onCompleted: {
                currentIndex = model.indexOf(dataObject.pointsPerOctave);
            }
            onCurrentIndexChanged: {
                dataObject.pointsPerOctave = model[currentIndex];
            }
        }

        RowLayout {
            Layout.fillWidth: true
        }

        Root.TitledCombo {
            tooltip: qsTr("show only this source")
            model: SourceModel {
                addNone: true
                list: sourceList
            }
            Layout.preferredWidth: 280
            currentIndex: { model.indexOf(dataObject.filter) }
            textRole: "title"
            valueRole: "source"
            onCurrentIndexChanged: {
                dataObject.filter = model.get(currentIndex);
            }
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
