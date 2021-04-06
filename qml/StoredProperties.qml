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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.13
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.12

import OpenSoundMeter 1.0

Item {
    property var dataObject
    property string saveas: "osm"

    RowLayout
    {
        anchors.fill: parent

        ColumnLayout {
            Layout.fillHeight: true

            RowLayout {
                FloatSpinBox {
                    id: gainSpinBox
                    implicitWidth: 170
                    value: dataObject.gain
                    from: -30
                    to: 30
                    units: "dB"
                    onValueChanged: dataObject.gain = value
                    tooltiptext: qsTr("adjust gain")
                    Layout.alignment: Qt.AlignVCenter
                }

                FloatSpinBox {
                    id: delaySpinBox
                    implicitWidth: 170
                    value: dataObject.delay
                    from: -100
                    to: 100
                    units: "ms"
                    onValueChanged: dataObject.delay = value
                    tooltiptext: qsTr("adjust delay")
                    Layout.alignment: Qt.AlignVCenter
                }

                ColorPicker {
                    id: colorPicker

                    Layout.preferredWidth: 25
                    Layout.preferredHeight: 25
                    Layout.margins: 5

                    onColorChanged: {
                        dataObject.color = color
                    }
                    Component.onCompleted: {
                        colorPicker.color = dataObject.color
                    }
                }

                TextField {
                    placeholderText: qsTr("title")
                    width: 135
                    text: dataObject.name
                    onTextEdited: dataObject.name = text
                }
            }
            RowLayout {

                CheckBox {
                    text: qsTr("inverse")
                    Layout.leftMargin: 40
                    Layout.rightMargin: 40
                    implicitWidth: 90
                    checked: dataObject.inverse
                    onCheckStateChanged: dataObject.inverse = checked

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("inverse magnitude data")
                }

                CheckBox {
                    text: qsTr("polarity")
                    Layout.leftMargin: 40
                    Layout.rightMargin: 40
                    implicitWidth: 90
                    checked: dataObject.polarity
                    onCheckStateChanged: dataObject.polarity = checked

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("inverse polarity")
                }

                ComboBox {
                    displayText: qsTr("Save data as");

                    implicitWidth: 170
                    model: ["osm", "cal", "txt", "frd"]

                    onActivated: function() {
                        saveas = currentText;
                        fileDialog.open();
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("export data")
                }
            }
        }

        ScrollView {
            id: scrollTextArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.vertical.interactive: false

            TextArea {
                id:ta
                padding: 5
                placeholderText: qsTr("notes")
                text: dataObject.notes;
                onTextChanged: dataObject.notes = text;
                font.italic: true
                wrapMode: TextEdit.WrapAnywhere
                background: Rectangle{
                    height: scrollTextArea.height
                    width:  scrollTextArea.width
                    border.color: ta.activeFocus ? ta.Material.accentColor : ta.Material.hintTextColor
                    border.width: ta.activeFocus ? 2 : 1
                    color: "transparent"
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        selectExisting: false
        title: "Please choose a file's name"
        folder: (typeof shortcuts !== 'undefined' ? shortcuts.home : Filesystem.StandardFolder.Home)
        defaultSuffix: saveas
        onAccepted: {
            switch (saveas) {
                case "osm":
                    dataObject.save(fileDialog.fileUrl);
                    break;
                case "cal":
                    dataObject.saveCal(fileDialog.fileUrl);
                    break;
                case "txt":
                    dataObject.saveTXT(fileDialog.fileUrl);
                    break;
                case "frd":
                    dataObject.saveFRD(fileDialog.fileUrl);
                    break;
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
