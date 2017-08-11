import QtQuick 2.0
import QtCharts 2.2

Item {
    //anchors.fill: parent

    ChartView {
        //title: qsTr("Two Series, Common Axes")
        anchors.fill: parent
        legend.visible: false
        antialiasing: true

        ValueAxis {
            id: axisY
            min: -120
            max: 0
        }

        LogValueAxis {
        //ValueAxis {
            id: axisX
            min: 20
            max: 22000
        }

        LineSeries {
            id: series1

            property var m : measureModel
            name: m.name

            axisX: axisX
            axisY: axisY

            visible: m.active
            color: m.color
            width: 2

            Connections {
                target: series1.m
                onReadyRead: {
                    series1.m.updateRTASeries(series1);
                }
            }
        }
    }

    PropertiesOpener {
        propertiesQml: "qrc:/ChartProperties.qml"
    }
}
