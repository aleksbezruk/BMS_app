import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import BMS

ApplicationWindow {
    width: Screen.width
    height: Screen.height
    visible: true
    title: qsTr("BMS Application")

    property BleConnection bleConnection: null

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        // Pages
        SwipeView {
            id: pages
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // Page with BMS devices
            Page {
                contentItem: RowLayout {
                    spacing: 0
                    // Scanned BMS devices (Left panel)
                    ColumnLayout {
                        // anchors.fill: parent
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: parent.width * 0.2   // ← important (stretch factor)
                        spacing: 20
                        Label {
                            text: "Devices"
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.margins: 12
                        }
                        Button {
                            text: "Scan BMS devices"
                            spacing: 20
                            anchors.margins: 12
                            width: parent.width * 0.2
                            onClicked: bleManager.startScan()
                        }
                        Button {
                            text: "Stop Scan"
                            spacing: 20
                            width: parent.width * 0.2
                            onClicked: {
                                bleManager.stopScan()
                                bmsDevices.clear()
                            }
                        }
                    }
                    Connections {
                        target: bleManager
                        function isBMSdevice(name) {
                            if ((name === "QN9080_BMS") ||
                                (name === "BMS_MCU"))
                            {
                                return true
                            }
                            return false
                        }
                        function onDeviceFound(address, name) {
                            if (isBMSdevice(name)) {
                                bmsDevices.append({ address: address, name: name })
                            }
                        }
                    }

                    // BMS devices model
                    ListModel {
                        id: bmsDevices
                    }

                    // Scrollable list
                    ListView {
                        id: bmsList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: parent.width * 0.3
                        model: bmsDevices
                        clip: true
                        spacing: 6

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        delegate: Rectangle {
                            width: bmsList.width
                            height: 56
                            radius: 6
                            color: index % 2 ? "#202020" : "#2a2a2a"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4

                                Text {
                                    text: name
                                    color: "white"
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: address
                                    color: "#aaaaaa"
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }

                    // Visual Separator
                    ToolSeparator {
                        orientation: Qt.Vertical     // line top→bottom
                        Layout.fillHeight: true     // stretch vertically
                        Layout.preferredWidth: 20    // optional
                    }

                    // Connect BMS devices (right part)
                    ColumnLayout {
                        // anchors.fill: parent
                        id: rightPanel
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: parent.width * 0.55
                        spacing: 20
                        Label {
                            text: "Connect BMS devices"
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.margins: 12
                        }
                        Button {
                            text: "Connect to device"
                            width: parent.width * 0.2
                            onClicked: devicePopup.open()
                        }
                        Row {
                            spacing: 8
                            Image {
                                source: "images/battery.png"
                                width: 122
                                height: 122
                                visible: bleConnection?.isConnected ? true: false
                            }
                            Text {
                                text: bleConnection?.isConnected ? "BMS connected" : "Disconnected"
                                color: "magenta"
                            }
                        }
                        Connections {
                            target: bleConnection
                            ignoreUnknownSignals: true

                            function onConnectedChanged() {
                                if (!bleConnection)
                                    return

                                if (bleConnection.isConnected) {
                                    console.log("BLE connected")
                                    toast.show("BMS connected")
                                } else {
                                    console.log("BLE disconnected")
                                    toast.show("BMS disconnected")
                                }
                            }
                        }
                        Popup {
                            id: devicePopup
                            width: parent.width * 0.8
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
                                    model: bmsDevices   // 👈 SAME model

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

                                                // ---- destroy previous connection safely ----
                                                if (bleConnection) {
                                                    console.log("Destroying previous BLE connection")
                                                    bleConnection.disconnectDevice()
                                                    bleConnection.destroy()
                                                    bleConnection = null
                                                }

                                                // ---- create new connection ----
                                                bleConnection = Qt.createQmlObject(`
                                                    import BMS
                                                    BleConnection {}
                                                `, rightPanel)

                                                console.log("Created BleConnection for", address)

                                                bleConnection.error.connect((err) => {
                                                    console.log("BLE error:", err)
                                                })

                                                // ---- start connection ----
                                                bleConnection.connectToDevice(address, name)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Popup {
                            id: toast
                            x: parent.width/2 - width/2
                            y: parent.height - 80
                            width: 300
                            height: 50
                            modal: false
                            focus: false
                            background: Rectangle {
                                color: "#333"
                                radius: 6
                            }

                            Label {
                                anchors.centerIn: parent
                                text: toast.message
                                color: "magenta"
                            }

                            property string message: ""

                            function show(msg) {
                                message = msg
                                open()
                                timer.restart()
                            }

                            Timer {
                                id: timer
                                interval: 4000
                                onTriggered: toast.close()
                            }
                        }
                    }
                }
            }

            // Page with debug information
            Page {
                Connections {
                    target: bleManager
                    function deviceExists(address) {
                        for (var i = 0; i < bleDevicesModel.count; ++i) {
                            if (bleDevicesModel.get(i).address === address)
                                return true
                        }
                        return false
                    }
                    function onDeviceFound(address, name) {
                        if (!deviceExists(address) &&
                            ((name === "QN9080_BMS") ||
                            (name === "BMS_MCU"))) {
                            bleDevicesModel.append({ address: address, name: name })
                        }
                    }
                }
                Connections {
                    target: bleConnection
                    ignoreUnknownSignals: true

                    function onConnectedChanged() {
                        if (!bleConnection)
                            return
                        if (bleConnection.isConnected) {
                            bleDevicesModel.append({ address: "", name: "BMS connected" })
                        } else {
                            bleDevicesModel.append({ address: "", name: "BMS disconnected" })
                        }
                    }
                }
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    // Title
                    Label {
                        text: "Logs"
                        font.pixelSize: 18
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 12
                    }

                    // Clear button
                    Button {
                        text: "Clear Log"
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 12
                        onClicked: bleDevicesModel.clear()
                    }

                    // BLE devices model
                    ListModel {
                        id: bleDevicesModel
                    }

                    // Scrollable list
                    ListView {
                        id: bleList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: bleDevicesModel
                        clip: true
                        spacing: 6

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        delegate: Rectangle {
                            width: bleList.width/3
                            height: 56
                            radius: 6
                            color: index % 2 ? "#202020" : "#2a2a2a"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4

                                Text {
                                    text: name
                                    color: "white"
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: address
                                    color: "#aaaaaa"
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }
        }

        // Page indicator
        PageIndicator {
            id: indicator
            Layout.alignment: Qt.AlignHCenter
            count: pages.count
            currentIndex: pages.currentIndex
        }

        // Tabs at the bottom to switch Pages
        TabBar {
            id: tabBar
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 12

            TabButton {
                text: "BMS Devices"
                implicitWidth: 120

                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "white" : "lightgray"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }

                background: Rectangle {
                    color: parent.checked ? "magenta" : "black"
                }
            }

            TabButton {
                text: "Debug Logs"
                implicitWidth: 120

                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "white" : "lightgray"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }

                background: Rectangle {
                    color: parent.checked ? "magenta" : "black"
                }
            }
        }
    }
}
