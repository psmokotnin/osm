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
import QtQuick.Controls.Material 2.13

import "../" as Root
import "../elements"
import SourceModel 1.0
import OpenSoundMeter 1.0

Item {
    property var dataObject
    readonly property int wideWidth: width / 4 - 2
    readonly property int middleWidth: width / 8 - 2
    readonly property int narrowWidth: width / 8 - 2

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            spacing: 4

            TitledCombo {
                tooltip: qsTr("show only this source")
                model: SourceModel {
                    unrollGroups: true
                    list: sourceList
                }
                Layout.preferredWidth: wideWidth
                currentIndex: {
                    model.indexOf(dataObject.meter.source)
                }
                textRole: "title"
                valueRole: "source"
                onCurrentIndexChanged: {
                    dataObject.meter.source = model.get(currentIndex);
                }
            }

            DropDown {
                Layout.preferredWidth: middleWidth

                model: dataObject.meter.availableTypes
                currentIndex: model.indexOf(dataObject.meter.type)

                onCurrentValueChanged: dataObject.meter.type = currentValue

                ToolTip.visible: hovered
                ToolTip.text: qsTr("type")
            }

            TitledCombo {
                id: type
                Layout.preferredWidth: narrowWidth
                model: ["dBFs", "SPL"]
                currentIndex: dataObject.meter.mode
                onCurrentIndexChanged: {
                    dataObject.meter.mode = currentIndex;
                }
                enabled: dataObject.meter.type !== "Time"  &&
                         dataObject.meter.type !== "THD+N" &&
                         dataObject.meter.type !== "Leq"   &&
                         dataObject.meter.type !== "Crest" &&
                         dataObject.meter.type !== "Gain"  &&
                         dataObject.meter.type !== "Delay"

                ToolTip.visible: hovered
                ToolTip.text: qsTr("scale")
            }

            DropDown {
                Layout.preferredWidth: narrowWidth
                model: dataObject.meter.availableCurves
                currentIndex: model.indexOf(dataObject.meter.curve)
                onCurrentValueChanged: dataObject.meter.curve = currentValue

                enabled: dataObject.meter.type !== "Time" &&
                         dataObject.meter.type !== "THD+N" &&
                         dataObject.meter.type !== "Gain" &&
                         dataObject.meter.type !== "Delay"

                ToolTip.visible: hovered
                ToolTip.text: qsTr("weighting")
            }

            DropDown {
                Layout.preferredWidth: narrowWidth

                model: dataObject.meter.availableTimes
                currentIndex: model.indexOf(dataObject.meter.time)
                onCurrentValueChanged: currentValue ? dataObject.meter.time = currentValue : {}

                enabled: dataObject.meter.type !== "Time" &&
                         dataObject.meter.type !== "THD+N" &&
                         dataObject.meter.type !== "Gain" &&
                         dataObject.meter.type !== "Delay"

                ToolTip.visible: hovered
                ToolTip.text: qsTr("integration time")
            }

            FloatSpinBox {
                id:threshold
                min: -150
                max: 150
                step: 1.0
                value: dataObject.meter.threshold
                tooltiptext: qsTr("warning threshold")
                onValueChanged: dataObject.meter.threshold = value
                Layout.preferredWidth: wideWidth
                units: "dB"
                enabled: dataObject.meter.type !== "Time" &&
                         dataObject.meter.type !== "THD+N" &&
                         dataObject.meter.type !== "Gain" &&
                         dataObject.meter.type !== "Delay"

                Connections {
                    target: dataObject.meter
                    function onThresholdChanged() {
                        threshold.value = dataObject.meter.threshold;
                    }
                }
            }
        }

        RowLayout {

            DropDown {
                Layout.preferredWidth: narrowWidth
                model: [1, 2, 3]
                currentIndex: dataObject.grid.model.rows - 1
                onCurrentIndexChanged: {
                    dataObject.grid.model.rows = currentIndex + 1;
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("meter rows")
            }

            DropDown {
                Layout.preferredWidth: narrowWidth
                model: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                currentIndex: dataObject.grid.model.columns - 1
                onCurrentIndexChanged: {
                    dataObject.grid.model.columns = currentIndex + 1;
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("meter columns")
            }

            Button {
                text: "peak"
                checkable: true
                checked: dataObject.meter.peakHold
                onCheckedChanged: dataObject.meter.peakHold = checked
                Material.background: parent.Material.background

                ToolTip.visible: hovered
                ToolTip.text: qsTr("hold on peak")
                enabled: dataObject.meter.type !== "Time"  &&
                         dataObject.meter.type !== "THD+N" &&
                         dataObject.meter.type !== "Delay"
            }

            Button {
                text: "reset"
                onClicked: dataObject.meter.reset()
                Material.background: parent.Material.background

                ToolTip.visible: hovered
                ToolTip.text: qsTr("reset peak")
                enabled: dataObject.meter.type !== "Time"  &&
                         dataObject.meter.type !== "THD+N" &&
                         dataObject.meter.type !== "Delay"
            }

        }
    }
}
