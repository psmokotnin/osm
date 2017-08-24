import QtQuick 2.7
import QtQuick.Controls 2.2

Item {
    StackView {
        id: chartStack
        anchors.fill: parent
        anchors.margins: 0
        initialItem: rtaChart
    }

    Component {
        id: rtaChart

        Chart {
            id: chart
            type: "RTA"
            //anchors.fill: parent
        }
    }

    ComboBox {
        anchors.top: parent.top
        anchors.right: parent.right
        model: ["RTA", "Magnitude", "Phase", "Impulse", "Scope"]
        currentIndex: 0
        onCurrentIndexChanged: {
            chartStack.clear();
            chartStack.push("Chart.qml", {"type": model[currentIndex]});
        }
    }
}
