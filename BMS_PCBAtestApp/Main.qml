import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 600
    height: 1000
    visible: true
    title: qsTr("BMS test App")

    Rectangle {
        id: searchBMS_dialog
        x: 8
        y: 36
        width: 570
        height: 195
        color: "#ffffff"
        anchors.fill: parent

        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#6f6969"
            }

            GradientStop {
                position: 1
                color: "#ae8b9c"
            }
            orientation: Gradient.Vertical
        }

        Button {
            id: scan_button
            text: qsTr("Scan")
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 8
            anchors.topMargin: 8
            highlighted: true
            font.styleName: "ExtraBold"
            font.pointSize: 14
            flat: false
            icon.color: "#454444"
            clip: false
            onClicked: bmsListModel.append({"name": "Device 1", "address": "192.168.0.10"})
        }

        ListView {
            id: listView
            x: 388
            y: 15
            width: 161
            height: 172
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 10
            anchors.topMargin: 10
            model: bmsListModel
            ListModel {
                id: bmsListModel
            }
            delegate: Rectangle {
                width: parent.width
                height: 60
                border.width: 1
                color: "grey"

                Column {
                    anchors.centerIn: parent

                    Text { text: name }
                    Text { text: address }
                }
            }
        }

        Button {
            id: stop_scan_button
            text: qsTr("Stop scan")
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 5
            anchors.topMargin: 45
            highlighted: true
            font.styleName: "ExtraBold"
            font.pointSize: 14
            onClicked: bmsListModel.clear()
        }
    }
}
