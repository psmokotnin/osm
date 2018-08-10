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
       propertiesQml: "qrc:/ChartProperties.qml"
       pushObject: fftChart
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
