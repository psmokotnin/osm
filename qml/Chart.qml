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
                        var newSeries = fftChart.appendDataSource(item.dataModel);
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
    }

    ComboBox {
        anchors.top: parent.top
        anchors.right: parent.right
        model: ["RTA", "Magnitude", "Phase", "Impulse", "Scope"]
        currentIndex: 0
        onCurrentIndexChanged: {
            fftChart.type = model[currentIndex];
        }
        Component.onCompleted: {
            fftChart.type = model[currentIndex];
        }
    }
}
