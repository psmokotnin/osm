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
import FftChart 1.0

Item {
    id: chartview

    FftChart {
        id: fftChart
        anchors.fill: parent

        Component.onCompleted: {
            for (var i = 0;
                 i < applicationWindow.dataSourceList.list.model.count;
                 i ++
                 ) {
                    var item = applicationWindow.dataSourceList.list.model.get(i);

                    if (item.chartable) {
                        fftChart.appendDataSource(item.dataModel);
                    }
            }

            applicationWindow.dataSourceList.modelAdded.connect(function(item) {
                fftChart.appendDataSource(item.dataModel);
                fftChart.needUpdate();
            });

            applicationWindow.dataSourceList.modelRemoved.connect(function(item) {
                fftChart.removeDataSource(item.dataModel);
                fftChart.needUpdate();
            });
        }
    }

    PropertiesOpener {
        id: opener
        propertiesQml: "qrc:/ChartProperties.qml"
        pushObject: fftChart
        cursorShape: "CrossCursor";
        hoverEnabled: true
        onEntered: cursor.visible = true
        onExited: cursor.visible = false
    }

    Label {
        id: cursor
        text: "%1".arg(fftChart.y2v(opener.mouseY)) + "\n" + "%1".arg(fftChart.x2v(opener.mouseX))
        x: opener.mouseX + cursor.fontInfo.pixelSize / 2
        y: opener.mouseY - cursor.height / 2
        visible: opener.containsMouse
    }

    ComboBox {
        anchors.top: parent.top
        anchors.right: parent.right
        implicitWidth: 130
        model: ["RTA", "Magnitude", "Phase", "Impulse"/*, "Scope"*/]
        currentIndex: 0
        onCurrentIndexChanged: {
            fftChart.type = model[currentIndex];
        }
        Component.onCompleted: {
            fftChart.type = model[currentIndex];
        }
    }
}
