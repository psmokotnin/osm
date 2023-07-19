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
import QtQuick 2.12
import QtQuick.Layouts 1.12
import "../" as Root
import "../SPL" as SPL
import OpenSoundMeter 1.0;

Item {
    anchors.fill: parent

    GridLayout {
        id: grid
        rows: 1
        columns: 3

        flow: GridLayout.LeftToRight
        anchors.fill: parent
        columnSpacing: 5
        rowSpacing: 5

        property int itemWidth:  (width  - (columns - 1) * columnSpacing) / columns - 1
        property int itemHeight: (height - (rows    - 1) * rowSpacing   ) / rows - 1

        Repeater {
            model: grid.rows * grid.columns

            delegate: SPL.Meter {
                Layout.preferredHeight: grid.itemHeight
                Layout.preferredWidth: grid.itemWidth
                gridRef: grid
                dataSource: MeterPlot {
                    list: sourceList
                    source: sourceList.firstSource()
                }
            }
        }
    }
}
