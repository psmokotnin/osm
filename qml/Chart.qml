import QtQuick 2.7
import QtCharts 2.2
import QtQuick.Controls 2.2

Item {

    Label {
        text: chart.type
        z: 2
        anchors.top: chart.top
        anchors.topMargin: chart.margins.top

        anchors.right: chart.right
        anchors.rightMargin: chart.margins.right
    }

    ChartView {
        id: chart

        property var availableTypes : ["RTA", "Magnitude", "Phase"]
        property string type : availableTypes[0]
        property var allSeries : []

        anchors.fill: parent
        legend.visible: false
        antialiasing: true

        PropertiesOpener {
            propertiesQml: "qrc:/ChartProperties.qml"
            pushObject: chart
        }

        function axisByType(direction) {
            if (direction == "X")
                return createAxis("log", 20, 20000);
            else {//Y
                switch (type) {
                    case "Magnitude": return createAxis("lin", -18, 18);
                    case "Phase": return createAxis("lin", -180, 180);

                    default:
                    case "RTA": return createAxis("lin", -120, 0);
                }
            }
        }

        function appendSeries(dataModel) {

            var series = createSeries(ChartView.SeriesTypeLine,
                                      dataModel.name,
                                      axisByType("X"), axisByType("Y"));

            series.useOpenGL = true;

            dataModel.readyRead.connect(function() {
                dataModel.updateSeries(series, chart.type);
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
            allSeries.push(series);
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
                "import QtQuick 2.0;
                 import QtCharts 2.0; " +
                 typeItem + "{ min: " + min + "; max: " + max + " }",
                  chart
            );
        }

        onTypeChanged: {
            for (var i = 0; i < count; i ++) {
                setAxisX(axisByType("X"), series(i));
                setAxisY(axisByType("Y"), series(i));
            }
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
