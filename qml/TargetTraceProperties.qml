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
import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import "elements"

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: true

            FloatSpinBox {
                value: targetTraceModel.width
                from: -20
                to: 20
                step: 0.1
                units: "dB"
                indicators: false
                onValueChanged: targetTraceModel.width = value
                Layout.preferredWidth: firstSegmentStart.width
                tooltiptext: qsTr("width")
            }

            Rectangle {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignTop

            FloatSpinBox {
                id: firstSegmentStart
                value: targetTraceModel.start(0)
                from: -20
                to: 20
                step: 0.1
                units: "dB"
                indicators: false
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                onValueChanged: targetTraceModel.setStart(0, value)
                tooltiptext: qsTr("1 segment end")
            }

            FloatSpinBox {
                id: firstSegmentEnd
                value: targetTraceModel.end(0)
                from: -20
                to: 20
                step: 0.1
                units: "dB"
                indicators: false
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                onValueChanged: targetTraceModel.setEnd(0, value)
                tooltiptext: qsTr("1 segment end")
            }

            FloatSpinBox {
                value: targetTraceModel.point(0)
                from: 20
                to: 20000
                step: 1
                units: "Hz"
                indicators: false
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                decimals: 0
                onValueChanged: targetTraceModel.setPoint(0, value)
                tooltiptext: qsTr("1 turning-point")

                Rectangle {
                    anchors.fill: parent
                    color: Material.color(Material.Grey)
                    opacity: 0.3
                }
            }

            FloatSpinBox {
                value: targetTraceModel.start(1)
                from: -20
                to: 20
                step: 0.1
                units: "dB"
                indicators: false
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                onValueChanged: targetTraceModel.setStart(1, value)
                tooltiptext: qsTr("2 segment start")
            }

            FloatSpinBox {
                value: targetTraceModel.end(1)
                from: -20
                to: 20
                step: 0.1
                units: "dB"
                indicators: false
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                onValueChanged: targetTraceModel.setEnd(1, value)
                tooltiptext: qsTr("2 segment end")
            }

            FloatSpinBox {
                value: targetTraceModel.point(1)
                from: 20
                to: 20000
                step: 1
                units: "Hz"
                indicators: false
                decimals: 0
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                onValueChanged: targetTraceModel.setPoint(1, value)
                tooltiptext: qsTr("2 turning-point")

                Rectangle {
                    anchors.fill: parent
                    color: Material.color(Material.Grey)
                    opacity: 0.3
                }
            }

            FloatSpinBox {
                value: targetTraceModel.start(2)
                from: -20
                to: 20
                step: 0.1
                units: "dB"
                indicators: false
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                onValueChanged: targetTraceModel.setStart(2, value)
                tooltiptext: qsTr("3 segment start")
            }

            FloatSpinBox {
                value: targetTraceModel.end(2)
                from: -20
                to: 20
                step: 0.1
                units: "dB"
                indicators: false
                Layout.preferredWidth: 100
                Layout.fillWidth: true
                onValueChanged: targetTraceModel.setEnd(2, value)
                tooltiptext: qsTr("3 segment end")
            }

        }
    }

}
