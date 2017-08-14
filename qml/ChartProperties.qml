import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    id: chartProperties
    property var dataObject

    RowLayout {
        spacing: 0

        /*TitledCombo {
            title: qsTr("charts count")
            model: ["1", "2"]
        }*/

        TitledCombo {
            title: qsTr("chart type")
            model: chartProperties.dataObject.availableTypes
            currentIndex: chartProperties.dataObject.availableTypes.indexOf(chartProperties.dataObject.type)
            onCurrentIndexChanged: {
                chartProperties.dataObject.type = chartProperties.dataObject.availableTypes[currentIndex];
            }
        }
    }
}
