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
import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQml 2.2
import QtQml.Models 2.3
import QtQuick.Controls.Material 2.1

import SourceModel 1.0
/**
 * SideBar
 *
 * There are all current active measurements, stores and sound sources
 * at the sidebar item.
 *
 * For detail settings the bottom bar is used.
 */
Item {
    property Item list : sideList
    property int colorIndex: 6;

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        ComboBox {
            id: chartsCount
            Layout.alignment: Qt.AlignHCenter
            model: ["Single", "Double", "Three"]
            currentIndex: charts.count - 1
            onCurrentIndexChanged: {
                applicationWindow.charts.count = currentIndex + 1
            }
        }

        Generator {}

        Component {
            id: measurementDelegate
            Measurement {
                width: sideList.width
                dataModel: modelData
            }
        }
        Component {
            id: storedDelegate
            Stored {
                width: sideList.width
                dataModel: modelData
            }
        }
        ListView {
            id: sideList
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.margins: 5
            model: SourceModel {
                id: sourceModel
                list: sourceList
            }
            clip: true
            delegate: Component {
                Loader {
                    property var modelData: model.source
                    sourceComponent:
                        switch(model.name) {
                            case "Measurement": return measurementDelegate;
                            case "Stored": return storedDelegate;
                            default: console.log("unknow model " + model.name);return ;
                    }
                }
            }

            ScrollIndicator.vertical: ScrollIndicator {}
        }

    }
}
