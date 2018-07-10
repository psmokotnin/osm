import QtQuick 2.7
import QtQuick.Controls 2.2

Item {
    StackView {
        id: chartStack
        anchors.fill: parent
        anchors.margins: 0
        initialItem: chartview
    }

    Component {
        id: chartview

        Chart {
            id: chart
            anchors.fill: parent
        }
    }
}
