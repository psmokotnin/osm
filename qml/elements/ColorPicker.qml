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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.1

Item {
    id: picker
    property color color

    Rectangle {
        id: preview
        width: picker.width
        height: picker.height
        color: picker.color
    }

    MouseArea {
        property int mouseButtonClicked: Qt.NoButton
        anchors.fill: preview
        cursorShape: "PointingHandCursor"
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            if (pressedButtons & Qt.LeftButton) {
                mouseButtonClicked = Qt.LeftButton
            } else if (pressedButtons & Qt.RightButton) {
                mouseButtonClicked = Qt.RightButton
            }
        }
        onClicked: function(e) {
            if (mouseButtonClicked === Qt.LeftButton) {
                colorDialog.open()
            } else if (mouseButtonClicked === Qt.RightButton) {
                picker.color = sourceList.nextColor();
            }
        }
    }

//    ColorDialog {
//        id: colorDialog
//        title: qsTr("Please choose a color")
//        color: picker.color

//        onAccepted: {
//            picker.color = color
//        }
//    }

    Dialog {
        id: colorDialog
        modal: true
        focus: true
        parent: applicationWindow.contentItem

        contentWidth:  Math.min(applicationWindow.width, applicationWindow.height)
        contentHeight: Math.min(applicationWindow.width, applicationWindow.height) * 10 / 19

        x: (applicationWindow.width  - width) / 2
        y: (applicationWindow.height - height) / 2

        onAccepted: {}
        onRejected: {}

        contentItem: Item {
            id: palette
            focus: true

            visible: colorDialog.visible
            Keys.onReturnPressed: {
                colorDialog.rejected();
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 5
                Repeater {
                    id: shadeRepeater
                    model: [
                        Material.Shade50,
                        Material.Shade100,
                        Material.Shade200,
                        Material.Shade300,
                        Material.Shade400,
                        Material.Shade500,
                        Material.Shade600,
                        Material.Shade700,
                        Material.Shade800,
                        Material.Shade900,
                    ]

                    RowLayout {
                        Layout.fillHeight: true
                        property var baseShade : modelData
                        spacing: 5

                        Repeater {
                            id: colorRepeater
                            model: [
                                Material.Red,
                                Material.Pink,
                                Material.Purple,
                                Material.DeepPurple,
                                Material.Indigo,
                                Material.Blue,
                                Material.LightBlue,
                                Material.Cyan,
                                Material.Teal,
                                Material.Green,
                                Material.LightGreen,
                                Material.Lime,
                                Material.Yellow,
                                Material.Amber,
                                Material.Orange,
                                Material.DeepOrange,
                                Material.Brown,
                                Material.Grey,
                                Material.BlueGrey
                            ]

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.preferredWidth: 15
                                Layout.preferredHeight: 15
                                color: Material.color(modelData, baseShade);

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: "PointingHandCursor"
                                    onClicked: {
                                        picker.color = color
                                        colorDialog.close();
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
    }
}
