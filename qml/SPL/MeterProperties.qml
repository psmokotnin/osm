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

import "../" as Root
import "../elements"
import SourceModel 1.0
import OpenSoundMeter 1.0

Item {
    property var dataObject
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {

            TitledCombo {
                tooltip: qsTr("show only this source")
                model: SourceModel {
                    list: sourceList
                }
                Layout.preferredWidth: 205
                currentIndex: {
                    model.indexOf(dataObject.meter.source)
                }
                textRole: "title"
                valueRole: "source"
                onCurrentIndexChanged: {
                    dataObject.meter.source = model.get(currentIndex);
                }
            }

            TitledCombo {
                id: type
                Layout.preferredWidth: 100
                model: ["dBFs", "SPL"]
                currentIndex: dataObject.meter.mode
                onCurrentIndexChanged: {
                    dataObject.meter.mode = currentIndex;
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("scale")
            }

            DropDown {
                Layout.preferredWidth: 100
                model: dataObject.meter.availableCurves
                currentIndex: model.indexOf(dataObject.meter.curve)
                onCurrentValueChanged: dataObject.meter.curve = currentValue

                ToolTip.visible: hovered
                ToolTip.text: qsTr("weighting")
            }

            DropDown {
                Layout.preferredWidth: 100

                model: dataObject.meter.availableTimes
                currentIndex: model.indexOf(dataObject.meter.time)
                onCurrentValueChanged: dataObject.meter.time = currentValue

                ToolTip.visible: hovered
                ToolTip.text: qsTr("time")
            }

            FloatSpinBox {
                id:threshold
                min: -150
                max: 150
                step: 1.0
                value: dataObject.meter.threshold
                tooltiptext: qsTr("warning threshold")
                onValueChanged: dataObject.meter.threshold = value
                Layout.preferredWidth: 200
                units: "dB"

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
                Layout.preferredWidth: 100
                model: [1, 2, 3]
                currentIndex: dataObject.grid.rows - 1
                onCurrentIndexChanged: {
                    dataObject.grid.rows = currentIndex + 1;
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("meter rows")
            }

            DropDown {
                Layout.preferredWidth: 100
                model: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                currentIndex: dataObject.grid.columns - 1
                onCurrentIndexChanged: {
                    dataObject.grid.columns = currentIndex + 1;
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("meter columns")
            }

        }
    }
}
