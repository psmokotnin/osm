import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.1


ApplicationWindow {
    visible: true

    width: 600
    height: 300
    minimumWidth: 600
    minimumHeight: 300

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
                id: t
                y: 0
                x: 0

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: bottomtab.top
            }

            //Properties area
            Rectangle {
                id: bottomtab
                height: 100
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                color: "green"
            }
        }

        //Measures area
        SideBar{
            id: righttab
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 200
        }
    }
}
