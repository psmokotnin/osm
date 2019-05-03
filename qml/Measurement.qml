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
    height: 50
    width: (parent ? parent.width : 0)

    RowLayout {
        width: parent.width

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter
            checked: dataModel.active
            checkedColor: dataModel.color

            onCheckStateChanged: {
                dataModel.active = checked
            }
            error: dataModel.error
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text:  dataModel.name

                PropertiesOpener {
                   id:opener
                   propertiesQml: "qrc:/MeasurementProperties.qml"
                   pushObject: measurement.dataModel
                }
            }

            Meter {
                dBV: dataModel.level
                width: parent.width
            }

            Meter {
                dBV: dataModel.referenceLevel
                width: parent.width
            }
        }

        Connections {
            target: dataModel
            onColorChanged: checkbox.checkedColor = dataModel.color
        }

        Connections {
            target: dataModel
            onSampleRateChanged: {
                    var pb = applicationWindow.properiesbar;
                    var reopen = false;
                    if (pb.currentObject === dataModel) {
                        pb.reset();
                        reopen = true;
                    }
                    if (reopen) {
                        pb.open(dataModel, opener.propertiesQml);
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
