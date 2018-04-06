import QtQuick 2.7
import QtQuick.Controls 2.2
import FftChart 1.0

Item {
    StackView {
        id: chartStack
        anchors.fill: parent
        anchors.margins: 0
        //initialItem: rtaChart
    }

    Component {
        id: rtaChart

        Chart {
            id: chart
            type: "RTA"
            //anchors.fill: parent
        }
    }

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
                        //item.dataModel.updateSeries(newSeries, chart.type);
                    }
            }
            //TODO: add series on store
        }
    }

    ComboBox {
        anchors.top: parent.top
        anchors.right: parent.right
        model: ["RTA", "Magnitude", "Phase", "Impulse", "Scope"]
        currentIndex: 0
        onCurrentIndexChanged: {
            fftChart.type = model[currentIndex];
            //chartStack.clear();
            //chartStack.push("Chart.qml", {"type": model[currentIndex]});
        }
        Component.onCompleted: {
            fftChart.type = model[currentIndex];
        }
    }
}
