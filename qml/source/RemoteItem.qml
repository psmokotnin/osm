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
import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import "qrc:/"

Item {
    id: item
    property var dataModel;
    property var dataModelData : dataModel.data
    property bool chartable : true;
    property bool highlight : false;
    property string propertiesQml: "qrc:/source/RemoteItemProperties.qml"
    height: 50
    width: parent.width

    RowLayout {
        width: parent.width

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter

            checkedColor: (dataModel && dataModelData ? dataModelData.color : "")

            onCheckStateChanged: {
                dataModelData.active = checked
            }
            Component.onCompleted: {
                checked = dataModelData.active
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                font.bold: highlight
                text:  (dataModel && dataModelData ? dataModelData.name : "")
            }

            RowLayout {
                Rectangle {
                    id: indicator
                    width: 7
                    height: width
                    radius: width /2
                    color: updateColor()
                    function updateColor() {
                        if (dataModel && dataModelData) {
                            switch(dataModelData.state) {
                                case 1:
                                    return Material.color(Material.Orange);
                                case 2:
                                    return Material.color(Material.Green);
                            }
                        }
                        //error or unknown state
                        return Material.color(Material.Red);
                    }
                    Connections {
                        target: dataModelData
                        function onStateChanged() {
                            color = indicator.updateColor();
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    font.bold: highlight
                    font.pixelSize: 10
                    text:  (dataModel && dataModelData ? "@" + dataModelData.host : "")
                }
            }
        }

        Connections {
            target: dataModelData
            function onColorChanged() {
                checkbox.checkedColor = dataModelData.color;
            }
            function onActiveChanged() {
                checkbox.checked = dataModelData.active;
            }
        }
    }

    Button {
        anchors.right: parent.right
        anchors.top: parent.top

        font.family: "Osm"
        font.bold: false
        text: "\ue808"

        font.pixelSize: 12
        rightPadding: 31
        leftPadding: 4

        background: Rectangle {
            color: "transparent"
        }

        ToolTip.visible: hovered
        ToolTip.text: qsTr("refresh data")

        onClicked: dataModelData.refresh();
    }

    Component.onCompleted: {
        if (dataModelData.objectName === "RemoteMeasurement") {
            propertiesQml = "qrc:/source/MeasurementProperties.qml";
        } else if (dataModelData.objectName === "RemoteStored") {
            propertiesQml = "qrc:/source/StoredProperties.qml";
        }
    }
}
