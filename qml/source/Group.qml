/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import "qrc:/"

Item {
    id: group

    property var dataModel : [];
    property var sharedGroup : dataModel.data
    property bool chartable : false;
    property bool highlight : false;
    property string propertiesQml: "qrc:/source/GroupProperties.qml"

    width: parent.width
    height: 50

    RowLayout {
        width: parent.width

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter

            checkedColor: (sharedGroup ? sharedGroup.color : "")

            onCheckStateChanged: {
                if (sharedGroup) {
                    sharedGroup.active = checked
                }
            }
            Component.onCompleted: {
                checked = sharedGroup ? sharedGroup.active : false
            }

            Connections {
                target: sharedGroup
                function onColorChanged() {
                    checkbox.data.checkedColor = sharedGroup.color;
                }
                function onActiveChanged() {
                    checkbox.checked = sharedGroup ? sharedGroup.active : false;
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            RowLayout {
                Label {
                    font.family: "Osm"
                    text: "\uf114"
                }

                Label {
                    Layout.fillWidth: true
                    font.bold: highlight
                    text:  (sharedGroup ? sharedGroup.name : "")
                }
            }

            RowLayout {
                Layout.maximumHeight: 7

                Repeater {
                    model: sharedGroup ? sharedGroup.sourceList.count : 0

                    Rectangle {
                        property var source: sharedGroup ? sharedGroup.sourceList.get(index) : source
                        color: (source && source.data ? source.data.color : "transparent")
                        width: 7
                        height: 7
                        visible: (source ? true : false)
                    }
                }
            }

        }
    }
}
