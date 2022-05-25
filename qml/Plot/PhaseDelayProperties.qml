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
        anchors.fill: parent
        spacing: 0

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
                step: 1.0
                decimals: 0
                scale: dataObject.yScale
                value: dataObject.ymin
                onValueChanged: dataObject.ymin = value
                from: dataObject.yLowLimit
                to: dataObject.yHighLimit
                editable: true
                implicitWidth: 170
                Layout.fillWidth: true
                tooltiptext: qsTr("y from")
                units: dataObject.yLabel
            }

            FloatSpinBox {
                id: ymaxFloatBox
                step: 1.0
                decimals: 0
                scale: dataObject.yScale
                value: dataObject.ymax
                onValueChanged: dataObject.ymax = value
                min: dataObject.yLowLimit
                max: dataObject.yHighLimit
                implicitWidth: 170
                Layout.fillWidth: true
                tooltiptext: qsTr("y to")
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

            CheckBox {
                id: coherence
                text: qsTr("use coherence")
                Layout.fillWidth: true
                checked: dataObject.coherence
                onCheckStateChanged: dataObject.coherence = checked

                ToolTip.visible: hovered
                ToolTip.text: qsTr("use coherence as alpha channel")
            }

            FloatSpinBox {
                min: 0.0
                max: 1.0
                step: 0.05
                value: dataObject.coherenceThreshold
                tooltiptext: qsTr("coherence threshold")
                onValueChanged: dataObject.coherenceThreshold = value
                Layout.fillWidth: true
                visible: coherence.checked
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
