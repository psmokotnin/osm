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
import "elements"

Item {
    height: 40
    width: parent.width

    Row {
        height: 40

        Switch {
            anchors.verticalCenter: parent.verticalCenter

            checked: generatorModel.enabled
            onCheckedChanged: generatorModel.enabled = checked
        }

        Label {
            id: label
            anchors.verticalCenter: parent.verticalCenter
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
            height: 40
            color: "transparent"
        }

        FloatSpinBox {
            id: gainSpinBox
            value: generatorModel.gain
            from: -90
            to: 0
            editable: true
            onValueChanged: generatorModel.gain = value
            units: "dB"
            indicators: false
            decimals: 0
            step: 1
            width: 45
            height: 40
            fontSize: label.font.pixelSize
        }

        Shortcut {
            sequence: "Ctrl+G"
            onActivated: generatorModel.enabled = !generatorModel.enabled;
        }
    }
}
