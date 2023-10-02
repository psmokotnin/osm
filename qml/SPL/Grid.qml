/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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
import QtQuick 2.15
import QtQuick.Layouts 1.12
import "../" as Root
import "../SPL" as SPL
import OpenSoundMeter 1.0
import OpenSoundMeterModule 1.0

Item {
    id: control
    property alias settings: model.settings
    anchors.fill: parent

    TableView{
        id: view
        anchors.fill: parent
        columnSpacing: 2
        rowSpacing: 2
        clip: false
        reuseItems: true

        property int cellWidth:  Math.floor(width  / columns)  - columnSpacing
        property int cellHeight: Math.floor(height / rows)     - rowSpacing

        columnWidthProvider : function() {return control.width > 0 ? cellWidth : 1; }
        rowHeightProvider : function() { return control.height > 0 ? cellHeight : 1; }

        model: MeterTableModel {
            id: model
        }
        Component.onCompleted: {
            model.sourceList = sourceList;
            model.settings = control.settings;
        }
        delegate: SPL.Meter {
            gridRef: view
            dataSource: model.meter
        }

        Connections {
            target: control
            function onWidthChanged() {
                view.forceLayout();
            }
            function onHeightChanged() {
                view.forceLayout();
            }
        }
        Connections {
            target: model

            function onRowsChanged() {
                view.forceLayout();
            }
            function onColumnsChanged() {
                view.forceLayout();
            }
        }
    }
}
