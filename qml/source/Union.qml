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
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import "qrc:/"

Item {
    property var dataModel;
    property bool chartable : true;
    property bool highlight : false;
    property string propertiesQml: "qrc:/source/UnionProperties.qml"
    height: 50
    width: parent.width

    RowLayout {
        width: parent.width

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter

            checkedColor: (dataModel ? dataModel.color : "")

            onCheckStateChanged: {
                dataModel.active = checked
            }
            Component.onCompleted: {
                checked = dataModel.active
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                font.bold: highlight
                text:  (dataModel ? dataModel.name : "")
            }

            RowLayout {
                Layout.maximumHeight: 7

                Repeater {
                    model: dataModel ? dataModel.count : 0

                    Rectangle {
                        property var source: dataModel ? dataModel.getSource(index) : null
                        color: (source ? source.color : "transparent")
                        width: 7
                        height: 7
                        visible: (source ? true : false)

                        Connections {
                            target: dataModel
                            function onModelChanged() {
                                source = dataModel.getSource(index);
                            }
                        }
                    }
                }
            }

        }

        Connections {
            target: dataModel
            function onColorChanged() {
                checkbox.checkedColor = dataModel.color;
            }
            function onActiveChanged() {
                checkbox.checked = dataModel.active;
            }
        }
    }
}
