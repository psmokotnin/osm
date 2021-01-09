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

Item {
    id: measurement

    property var dataModel;
    property bool chartable : true;
    property bool highlight : false;
    property string propertiesQml: "qrc:/MeasurementProperties.qml"
    height: 50
    width: (parent ? parent.width : 0)

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: 10

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter
            checked: (dataModel ? dataModel.active : false)
            checkedColor: (dataModel ? dataModel.color : "none")

            onCheckStateChanged: {
                if (dataModel)
                    dataModel.active = checked
            }
            error: (dataModel ? dataModel.error : false)
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                font.bold: highlight
                text:  (dataModel ? dataModel.name : "")
            }

            Meter {
                dBV: (dataModel ? dataModel.level : 0)
                width: parent.width
            }

            Meter {
                dBV: (dataModel ? dataModel.referenceLevel : 0)
                width: parent.width
            }
        }

        Connections {
            target: dataModel
            function onColorChanged() {
                checkbox.checkedColor = dataModel.color;
            }
        }

        Connections {
            target: dataModel
            function onSampleRateChanged() {
                    var pb = applicationWindow.properiesbar;
                    var reopen = false;
                    if (pb.currentObject === dataModel) {
                        pb.reset();
                        reopen = true;
                    }
                    if (reopen) {
                        pb.open(dataModel, measurement.propertiesQml);
                    }
                }
        }

        Component.onCompleted: {
            if (!dataModel.isColorValid()) {
                dataModel.color = applicationWindow.dataSourceList.nextColor();
            }
            dataModel.errorChanged.connect(function(error) {
                if (error) {
                    applicationWindow.message.showError(qsTr("Can't start the %1.<br/>Device is not supported.").arg(dataModel.name));
                }
            });
        }
    }
}
