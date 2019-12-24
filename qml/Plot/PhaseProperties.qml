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

import "../" as Root

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
                value: dataObject.rotate
                onValueChanged: {
                    if (Math.abs(value) > 180) {
                        value -= Math.abs(value) / value * 360;
                    }
                    dataObject.rotate = value;
                }
                from: -360
                to: 360
                editable: true
                implicitWidth: 170
                Layout.fillWidth: true

                ToolTip.visible: hovered
                ToolTip.text: qsTr("rotate")

                textFromValue: function(value, locale) {
                    return Number(value) + "º"
                }

                valueFromText: function(text, locale) {
                    return parseInt(text)
                }
            }

            SpinBox {
                value: dataObject.range
                onValueChanged: dataObject.range = value;
                from: 0
                to: 360
                editable: true
                implicitWidth: 170
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
                text: qsTr("Save Image");
                implicitWidth: 120
                onClicked: fileDialog.open();
            }
        }
        RowLayout {
            spacing: 0

            Root.TitledCombo {
                title: qsTr("ppo")
                implicitWidth: 170
                model: [3, 6, 12, 24, 48]
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
                implicitWidth: 170
                checked: dataObject.coherence
                onCheckStateChanged: dataObject.coherence = checked

                ToolTip.visible: hovered
                ToolTip.text: qsTr("use coherence as alpha channel")
            }

            Root.FloatSpinBox {
                min: 0.0
                max: 1.0
                step: 0.05
                value: dataObject.coherenceThreshold
                tooltiptext: qsTr("coherence Threshold")
                onValueChanged: dataObject.coherenceThreshold = value
                implicitWidth: 170
                visible: coherence.checked
            }

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
