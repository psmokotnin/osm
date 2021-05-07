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
import "elements"

Item {
    height: 40
    width: parent.width

    RowLayout {
        anchors.fill: parent
        anchors.rightMargin: 10
        spacing: 0

        Switch {
            Layout.alignment: Qt.AlignCenter

            checked: generatorModel.enabled
            onCheckedChanged: generatorModel.enabled = checked
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
            value: generatorModel.gain
            onValueChanged: generatorModel.gain = value
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
        }

        Shortcut {
            sequence: "Ctrl+G"
            onActivated: generatorModel.enabled = !generatorModel.enabled;
        }
    }
}
