import QtQuick 2.7
import QtCharts 2.2

Item {
    //anchors.fill: parent

    ChartView {
        id: chart
        anchors.fill: parent
        legend.visible: false
        antialiasing: true

        LogValueAxis {
            id: axisX
            min: 20
            max: 22000
        }

        ValueAxis {
            id: axisY
            min: -120
            max: 0
        }

        PropertiesOpener {
            propertiesQml: "qrc:/ChartProperties.qml"
        }

        function appendSeries(dataModel) {

            var series = createSeries(ChartView.SeriesTypeLine,
                                      dataModel.name,
                                      axisX, axisY);


            dataModel.readyRead.connect(function() {
                dataModel.updateRTASeries(series);
            });

            //name
            dataModel.nameChanged.connect(function() {
                series.name = dataModel.name;
            });

            //visible
            series.visible = dataModel.active;
            dataModel.activeChanged.connect(function() {
                series.visible = dataModel.active;
            });

            //color
            series.color = dataModel.color;
            dataModel.colorChanged.connect(function() {
                series.color = dataModel.color;
            });
        }

        Component.onCompleted: {
            for (var i = 0;
                 i < applicationWindow.dataSourceList.model.count;
                 i ++
                 ) {
                    var item = applicationWindow.dataSourceList.model.get(i);

                    if (item.chartable) {
                        chart.appendSeries(item.dataModel);
                }
            }
        }
    }
}
