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
import "../elements"
import SourceModel 1.0
import OpenSoundMeter 1.0

Item {
    id: chartProperties
    property var dataObject

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            id: axis
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

            SelectableSpinBox {
                id: ymin
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

            SelectableSpinBox {
                id: ymax
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

            function onYLimitsChanged() {
                ymax.from  = dataObject.yLowLimit;
                ymax.to    = dataObject.yHighLimit;
                ymax.value = dataObject.ymax;
                ymin.from  = dataObject.yLowLimit;
                ymin.to    = dataObject.yHighLimit;
                ymin.value = dataObject.ymin;
            }

            Connections {
                target: dataObject
                function onYmaxChanged() {
                    axis.onYLimitsChanged();
                }
                function onYminChanged() {
                    axis.onYLimitsChanged();
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

            TitledCombo {
                id: mode
                property bool checkModePPO : false
                implicitWidth: 170
                title: qsTr("mode")
                tooltip: qsTr("render data as")
                model: ["line", "bars", "lines"]
                currentIndex: dataObject.mode;
                onCurrentIndexChanged: {
                    dataObject.mode = currentIndex;
                    if (checkModePPO) {
                        if (model[currentIndex] === "bars" && dataObject.pointsPerOctave === 0) {
                            dataObject.pointsPerOctave = 12;
                        } else if (model[currentIndex] === "line") {
                            dataObject.pointsPerOctave = 0;
                        }
                    }
                }
            }

            TitledCombo {
                title: qsTr("ppo")
                tooltip: qsTr("points per octave")
                visible: mode.model[mode.currentIndex] !== "lines"
                model: [1, 3, 6, 12, 24, 48, "off"]
                currentIndex: {
                    model.indexOf(dataObject.pointsPerOctave ? dataObject.pointsPerOctave : "off")
                }

                onCurrentIndexChanged: {
                    dataObject.pointsPerOctave = model[currentIndex] === "off" ? 0 : model[currentIndex];
                    if (model[currentIndex] === "off" && dataObject.mode === 1 /*bars*/) {
                        dataObject.mode = 0;
                    }
                }
            }

            TitledCombo {
                id: scale
                implicitWidth: 170
                title: qsTr("")
                tooltip: qsTr("Show values in")
                model: ["dBfs", "SPL", "phons"]
                currentIndex: dataObject.scale;
                onCurrentIndexChanged: {
                    dataObject.scale = currentIndex;
                }
            }

            CheckBox {
                id: peaks
                text: qsTr("hold peaks")
                Layout.fillWidth: true
                checked: dataObject.showPeaks
                onCheckStateChanged: dataObject.showPeaks = checked
                visible: mode.model[mode.currentIndex] !== "line"

                ToolTip.visible: hovered
                ToolTip.text: qsTr("show peaks")
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

    Component.onCompleted: {
        mode.checkModePPO = true;
    }
}
