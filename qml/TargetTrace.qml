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
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.12
import "elements"

Item {
    height: 50
    width: parent.width

    RowLayout {
        width: parent.width

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter

            checkedColor: (targetTraceModel ? targetTraceModel.color : "")

            onCheckStateChanged: {
                targetTraceModel.active = checked
            }
            Component.onCompleted: {
                checked = targetTraceModel.active
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text: qsTr("Target Trace")

                PropertiesOpener {
                   propertiesQml: "qrc:/TargetTraceProperties.qml"
                   onClicked: {
                       open();
                   }
                }
            }
        }

        Connections {
            target: targetTraceModel
            function onColorChanged() {
                checkbox.checkedColor = dataModel.color;
            }
        }
    }
}
