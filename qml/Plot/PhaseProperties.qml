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
                id: rotate
                value: dataObject.rotate
                onValueChanged: {
                    if (Math.abs(value) > 180) {
                        value -= Math.abs(value) / value * 360;
                    }
                    dataObject.rotate = value;
                }

                from: -360
                to: 360
                wrap: true
                editable: true
                Layout.fillWidth: true

                ToolTip.visible: hovered
                ToolTip.text: qsTr("rotate")

                textFromValue: function(value, locale) {
                    if (positivePeriod.currentIndex && value < 0) {
                        value += 360;
                    }
                    return Number(value) + "º"
                }

                valueFromText: function(text, locale) {
                    return parseInt(text)
                }

                Connections {
                    target: dataObject
                    function onPositivePeriodChanged() {
                        let v =  dataObject.rotate;
                        rotate.value = v + 0.1;
                        rotate.value = v;
                    }
                }
            }

            SelectableSpinBox {
                value: dataObject.range
                onValueChanged: dataObject.range = value;
                from: 0
                to: 360
                editable: true
                Layout.fillWidth: true

                ToolTip.visible: hovered
                ToolTip.text: qsTr("range")

                textFromValue: function(value, locale) {
                    return Number(value) + "º"
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
            TitledCombo {
                title: qsTr("ppo")
                tooltip: qsTr("points per octave")
                model: [1, 3, 6, 12, 24, 48]
                Layout.fillWidth: true
                currentIndex: {
                    var ppo = dataObject.pointsPerOctave;
                    model.indexOf(ppo);
                }
                onCurrentIndexChanged: {
                    var ppo = model[currentIndex];
                    dataObject.pointsPerOctave = ppo;
                }
            }

            ComboBox {
                id: positivePeriod
                model: ["±180º", "0..360º"]
                Layout.fillWidth: true
                onCurrentTextChanged: {
                    dataObject.positivePeriod = currentIndex == 1;
                }
                currentIndex: dataObject.positivePeriod
            }

            CheckBox {
                id: coherence
                text: qsTr("use coherence")
                checked: dataObject.coherence
                onCheckStateChanged: dataObject.coherence = checked
                Layout.fillWidth: true
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
                opacity: coherence.checked
                enabled: coherence.checked
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
