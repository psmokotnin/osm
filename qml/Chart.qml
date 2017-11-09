import QtQuick 2.7
import QtCharts 2.2
import QtQuick.Controls 2.2

ChartView {
    id: chart

    property string type : "RTA"

    legend.visible: false
    antialiasing: true

    PropertiesOpener {
        propertiesQml: "qrc:/ChartProperties.qml"
        pushObject: chart
    }

    function axisByType(direction) {
        if (direction == "X")
            switch (type) {
            case "Scope": return createAxis("lin", -10, 10);break;//ms
            case "Impulse": return createAxis("lin", -10, 10);break;//ms

            default:
                return createAxis("log", 20, 20000);
            }
        else {//Y
            switch (type) {
                case "Magnitude": return createAxis("lin", -18, 18);
                case "Phase": return createAxis("lin", -180, 180);
                case "Scope": return createAxis("lin", -1, 1);
                case "Impulse": return createAxis("lin", -1, 1);

                default:
                case "RTA": return createAxis("lin", -120, 0);
            }
        }
    }

    function appendSeries(dataModel) {

        var aX = chart.count ? chart.axisX(chart.series[0]) : axisByType("X");
        var aY = chart.count ? chart.axisY(chart.series[0]) : axisByType("Y");
        var series = createSeries(ChartView.SeriesTypeLine,
                                  dataModel.name,
                                  aX, aY);

        series.useOpenGL = true;

        dataModel.readyRead.connect(function() {
            if (chart) {
                dataModel.updateSeries(series, chart.type);
            } else {
                return -1;
            }
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
        return series;
    }

    function createAxis(type, min, max) {
        var typeItem;
        switch (type) {
            case "log":
                typeItem = "LogValueAxis";
                break;

            default:
            case "lin":
                typeItem = "ValueAxis";
                break;
        }

        return Qt.createQmlObject(
            "import QtQuick 2.7;
             import QtCharts 2.2; " +
             typeItem + "{ min: " + min + "; max: " + max + " }",
              chart
        );
    }

    onTypeChanged: {
    }

    Component.onCompleted: {
        for (var i = 0;
             i < applicationWindow.dataSourceList.list.model.count;
             i ++
             ) {
                var item = applicationWindow.dataSourceList.list.model.get(i);

                if (item.chartable) {
                    var newSeries = chart.appendSeries(item.dataModel);
                    item.dataModel.updateSeries(newSeries, chart.type);
                }
        }
        applicationWindow.dataSourceList.modelAdded.connect(function(item) {
            if (chart && item.chartable) {
                var newSeries = chart.appendSeries(item.dataModel);
                item.dataModel.updateSeries(newSeries, chart.type);
            }
        });
    }
}
