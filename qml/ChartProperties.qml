import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {

    RowLayout {
        spacing: 0

        /*TitledCombo {
            title: qsTr("charts count")
            model: ["1", "2"]
        }*/

        TitledCombo {
            title: qsTr("chart type")
            model: ["RTA"]
            currentIndex: 0
        }
    }
}
