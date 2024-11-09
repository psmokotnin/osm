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
import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.2
import "elements"

Item {
    property bool remoteControlled : remoteClient.controlledGenerator && remoteClient.controlledGenerator.data
    property var currentGenerator : (remoteClient.controlledGenerator && remoteClient.controlledGenerator.data ? remoteClient.controlledGenerator.data : generatorModel)


    height: remoteControlled ? 40 : 57
    width: parent.width

    ColumnLayout {
        anchors.fill: parent
        anchors.rightMargin: 10

        RowLayout {
            spacing: 0
            Layout.preferredHeight: 40

            Switch {
                id: onoff
                Layout.alignment: Qt.AlignCenter

                checked: currentGenerator.enabled
                onCheckedChanged: currentGenerator.enabled = checked
            }

            Label {
                id: label
                Layout.alignment: Qt.AlignCenter
                text:  qsTr("Generator")

                PropertiesOpener {
                   propertiesQml: "qrc:/GeneratorProperties.qml"
                   onClicked: {
                       open();
                   }
                }
            }

            Rectangle {
                width: 15
                height: label.implicitHeight
                color: "transparent"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
            }

            FloatSpinBox {
                id: gainSpinBox
                value: currentGenerator.gain
                onValueChanged: currentGenerator.gain = value
                Layout.alignment: Qt.AlignCenter

                decimals: 0
                from: -90
                step: 1
                to: 0
                units: "dB"

                editable: true
                indicators: false
                background: false

                width: 45
                bottomPadding: 8
                fontSize: label.font.pixelSize

                Connections {
                    target: currentGenerator
                    function onGainChanged() {
                        gainSpinBox.value = currentGenerator.gain;
                    }
                }
            }

            Shortcut {
                sequence: "Ctrl+G"
                onActivated: currentGenerator.enabled = !currentGenerator.enabled;
            }
        }

        RowLayout {
            enabled: remoteControlled
            visible: remoteControlled

            Item {
                Layout.preferredWidth: 42
            }

            Rectangle {
                id: indicator
                width: 7
                height: width
                radius: width /2
                color: updateColor()
                function updateColor() {
                    if (remoteControlled && currentGenerator) {
                        switch(currentGenerator.state) {
                            case 1:
                                return Material.color(Material.Red);
                            case 2:
                                return Material.color(Material.Green);
                        }
                    }
                    //error or unknown state
                    return Material.color(Material.Red);
                }
                Connections {
                    target: remoteControlled ? currentGenerator : null
                    function onStateChanged() {
                        color = indicator.updateColor();
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                font.pixelSize: 10
                text:  (remoteControlled && currentGenerator ? "@" + currentGenerator.host : "")
            }
        }

    }
}
