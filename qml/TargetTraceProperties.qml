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
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.1
import "elements"

Item {
    readonly property int elementWidth: width / (targetTraceModel.size() * 2)

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
                Layout.preferredWidth: elementWidth * 2
                tooltiptext: qsTr("width")
            }

            Rectangle {
                Layout.preferredWidth: elementWidth * 2
            }

            FloatSpinBox {
                value: targetTraceModel.offset
                from: 40
                to: 1400
                step: 0.1
                units: "dB"
                indicators: false
                onValueChanged: targetTraceModel.offset = value
                Layout.preferredWidth: elementWidth * 2
                tooltiptext: qsTr("SPL offset")
            }

            Rectangle {
                Layout.fillWidth: true
            }

            DropDown {
                id: modeSelect
                model: targetTraceModel.presets
                currentIndex: targetTraceModel.preset
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Preset")
                onCurrentIndexChanged: targetTraceModel.preset = currentIndex;
                Layout.preferredWidth: 200
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignTop

            Repeater {
                model: targetTraceModel.size() < 8 ? targetTraceModel.size() : 7

                RowLayout {
                    Layout.fillWidth: true

                    FloatSpinBox {
                        id: frequency
                        value: targetTraceModel.frequency( index )
                        from: 20
                        to: 40000
                        step: 1
                        units: "Hz"
                        indicators: false
                        Layout.preferredWidth: elementWidth
                        Layout.fillWidth: true
                        decimals: 0
                        onValueChanged: targetTraceModel.setFrequency(index, value)
                        tooltiptext: qsTr("frequency " + (index + 1))

                        Rectangle {
                            anchors.fill: parent
                            color: Material.color(Material.Grey)
                            opacity: 0.3
                        }
                        Connections {
                            target: targetTraceModel
                            function onChanged() {
                                frequency.value = targetTraceModel.frequency( index );
                            }
                        }
                    }

                    FloatSpinBox {
                        id: gain
                        value: targetTraceModel.gain( index )
                        from: -20
                        to: 20
                        step: 0.1
                        decimals: 1
                        units: "dB"
                        indicators: false
                        Layout.preferredWidth: elementWidth
                        Layout.fillWidth: true
                        onValueChanged: targetTraceModel.setGain( index , value)
                        tooltiptext: qsTr("gain " + (index + 1))

                        Connections {
                            target: targetTraceModel
                            function onChanged() {
                                gain.value = targetTraceModel.gain( index );
                            }
                        }
                    }
                }
            }
        }
    }

}
