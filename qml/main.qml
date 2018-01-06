import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.1


ApplicationWindow {
    id:applicationWindow

    property alias properiesbar: bottomtab
    property alias charts: charts
    property alias dataSourceList : righttab

    visible: true

    width: 1000
    height: 600
    minimumWidth: 1000
    minimumHeight: 600

    Material.theme: Material.Light
    Material.accent: Material.Indigo

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            y: 0

            anchors.left: parent.left
            anchors.right: righttab.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            spacing: 0

            //Charts area
            Charts {
                id: charts
                y: 0
                x: 0

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: bottomtab.top
            }

            //Properties area
            PropetiesBar {
                id: bottomtab
                height: 120
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
            }
        }

        //Measures area
        SideBar {
            id: righttab

            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            width: 200
        }
    }
}
