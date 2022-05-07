/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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

        SelectableSpinBox {
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

        SelectableSpinBox {
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

        FloatSpinBox {
            id: yminFloatBox
            min: dataObject.yLowLimit
            max: dataObject.yHighLimit
            value: dataObject.ymin
            tooltiptext: qsTr("y from")
            onValueChanged: dataObject.ymin = value
            implicitWidth: 170
            Layout.fillWidth: true
            units: dataObject.yLabel
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
            units: dataObject.yLabel
        }
        Connections {
            target: dataObject
            function onYminChanged() {
                yminFloatBox.value = dataObject.ymin;
            }
            function onYmaxChanged() {
                ymaxFloatBox.value = dataObject.ymax;
            }
            function onThresholdChanged() {
                threshold.value = dataObject.threshold;
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

        TitledCombo {
            title: qsTr("ppo")
            tooltip: qsTr("points per octave")
            Layout.fillWidth: true
            model: [1, 3, 6, 12, 24, 48]
            currentIndex: {
                var ppo = dataObject.pointsPerOctave;
                model.indexOf(ppo);
            }
            onCurrentIndexChanged: {
                var ppo = model[currentIndex];
                dataObject.pointsPerOctave = ppo;
            }
        }

        Rectangle {
            width: 5
        }

        DropDown {
            id: type
            Layout.fillWidth: true
            model: ["normal", "squared", "SNR"]
            currentIndex: dataObject.type
            ToolTip.visible: hovered
            ToolTip.text: qsTr("value type")
            onCurrentIndexChanged: dataObject.type = currentIndex;
        }

        CheckBox {
            Layout.margins: 5
            Layout.leftMargin: 25
            Layout.bottomMargin: 10
            Layout.preferredWidth: 25
            Layout.preferredHeight: 25
            checked: dataObject.showThreshold
            onCheckStateChanged: dataObject.showThreshold = checked

            ToolTip.visible: hovered
            ToolTip.text: qsTr("show help line")
        }

        ColorPicker {
            id: colorPicker

            Layout.preferredWidth: 25
            Layout.preferredHeight: 25
            Layout.margins: 5

            onColorChanged: {
                dataObject.thresholdColor = color
            }
            Component.onCompleted: {
                colorPicker.color = dataObject.thresholdColor
            }
        }

        FloatSpinBox {
            id:threshold
            min: 0.0
            max: dataObject.ymax
            step: 0.05
            value: dataObject.threshold
            tooltiptext: qsTr("coherence threshold")
            onValueChanged: dataObject.threshold = value
            Layout.fillWidth: true
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
