import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 600
    height: 1000
    visible: true
    title: qsTr("BMS test App")

    property BleConnection bleConnection: null

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
            //onClicked: bmsListModel.append({"name": "Device 1", "address": "192.168.0.10"})
            onClicked: bleManager.startScan()
        }

        // BLE scan callbacks
        Connections {
            target: bleManager

            function isBMSdevice(name) {
                return (name === "QN9080_BMS" || name === "BMS_MCU")
            }
            function deviceExists(address) {
                for (var i = 0; i < bmsListModel.count; ++i) {
                    if (bmsListModel.get(i).address === address)
                        return true
                }
                return false
            }
            function onDeviceFound(address, name, vbatLvl) {
                if (!deviceExists(address) && isBMSdevice(name)) {
                    bmsListModel.append({ address: address, name: name })
                }
            }
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
            anchors.topMargin: 60
            highlighted: true
            font.styleName: "ExtraBold"
            font.pointSize: 14
            onClicked: bmsListModel.clear()
        }

        Button {
            id: connect_button
            text: qsTr("Connect")
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 5
            anchors.topMargin: 118
            highlighted: true
            font.styleName: "ExtraBold"
            font.pointSize: 14
            onClicked: devicePopup.open()
        }

        Button {
            id: disconnect_button
            text: qsTr("Disconnect")
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 5
            anchors.topMargin: 167
            font.styleName: "ExtraBold"
            font.pointSize: 14
            highlighted: true
            onClicked: bleConnection.disconnectDevice()
        }

        // ================= DEVICE POPUP =================
        Popup {
            id: devicePopup
            width: parent.width * 0.4
            height: parent.height * 0.7
            modal: true
            focus: true
            anchors.centerIn: parent

            Rectangle {
                anchors.fill: parent
                color: "#303030"
                radius: 8

                ListView {
                    anchors.fill: parent
                    model: bmsListModel

                    delegate: Rectangle {
                        width: devicePopup.width
                        height: 60
                        color: "#444"

                        Text {
                            anchors.centerIn: parent
                            text: name + " (" + address + ")"
                            color: "white"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {

                                devicePopup.close()

                                if (bleConnection) {
                                    console.log("Destroying previous BLE connection")
                                    bleConnection.disconnectDevice()
                                    bleConnection.destroy()
                                    bleConnection = null
                                }

                                bleConnection = Qt.createQmlObject(`
                                    import BMS_PCBAtestApp
                                    BleConnection {}
                                `, searchBMS_dialog)

                                console.log("Created BleConnection for", address)

                                bleConnection.error.connect((err) => {
                                    console.log("BLE error:", err)
                                })

                                bleConnection.connectToDevice(address, name)
                            }
                        }
                    }
                }
            }
        }
    }
}
