import QtQuick 2.0
import QtCharts 2.2

Item {
    //anchors.fill: parent

    //![1]
    ChartView {
        //title: qsTr("Two Series, Common Axes")
        anchors.fill: parent
        legend.visible: false
        antialiasing: true

        ValueAxis {
            id: axisY
            min: -96
            max: 0
        }

        LogValueAxis {
        //ValueAxis {
            id: axisX
            min: 20
            max: 22000
        }

        LineSeries {
            name: qsTr("n")
            id: series1

            axisX: axisX
            axisY: axisY

            property var m : measureModel

            Connections {
                target: series1.m
                onReadyRead: {
                    series1.m.updateRTASeries(series1);
                }
            }
        }


    }

    //measureModel.readyRead: {
    // Add data dynamically to the series
    Component.onCompleted: {
        for (var i = 1; i <= 20000; i+= 500) {
            series1.append(i, Math.random() * -96 );
            //series2.append(i, Math.random());
        }
    }
    //![1]

    PropertiesOpener {
        propertiesQml: "qrc:/ChartProperties.qml"
    }
}
