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
        Rectangle {
            id: pcbaTrim_Dialog
            height: 217
            visible: bleConnection?.isConnected? true: false
            color: "#8f7a83"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            anchors.bottomMargin: 15

            Rectangle {
                id: pcbaTrim_modeDialog
                height: 25
                width: 30
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 12
                TextInput {
                    id: modeInput
                    visible: true
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Rectangle {
                id: pcbaTrim_adcErrorDialog
                height: 25
                width: 30
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 37
                TextInput {
                    id: adcError_input
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Rectangle {
                id: pcbaTrim_bank1Dialog
                height: 25
                width: 100
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 62
                TextInput {
                    id: b1Ratio_input
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Rectangle {
                id: pcbaTrim_bank2Dialog
                height: 25
                width: 100
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 88
                TextInput {
                    id: b2Ratio_input
                    width: 80
                    height: 20
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Rectangle {
                id: pcbaTrim_bank3Dialog
                height: 25
                width: 100
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 114
                TextInput {
                    id: b3Ratio_input
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Rectangle {
                id: pcbaTrim_bank4Dialog
                height: 25
                width: 100
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 140
                TextInput {
                    id: b4Ratio_input
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Rectangle {
                id: pcbaTrim_adcIntDialog
                height: 25
                width: 30
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 166
                TextInput {
                    id: adcInt_input
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Rectangle {
                id: pcbaTrim_advIntDialog
                height: 25
                width: 60
                visible: bleConnection?.isConnected? true: false
                color: "lightgrey"
                border.color: "black"
                border.width: 1
                radius: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 175
                anchors.topMargin: 192
                TextInput {
                    id: advInt_input
                    text: qsTr("?")
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    readOnly: false
                    selectedTextColor: "#0e0d0d"
                    overwriteMode: true
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectionColor: "#f9f9fe"
                    clip: false
                    font.styleName: "ExtraBold"
                }
            }

            Text {
                id: mode_text
                width: 127
                height: 23
                text: qsTr("Set PCBA mode")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 14
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Text {
                id: adcError_text
                width: 116
                height: 22
                text: qsTr("Set ADC error")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 36
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Text {
                id: b1ConvRatio_text
                width: 133
                height: 20
                text: qsTr("Bank1 conv ratio")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 64
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Text {
                id: b2ConvRatio_text
                x: 20
                y: -682
                width: 133
                height: 20
                text: qsTr("Bank2 conv ratio")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 90
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Text {
                id: b3ConvRatio_text
                x: 20
                y: -656
                width: 133
                height: 20
                text: qsTr("Bank3 conv ratio")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 116
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Text {
                id: b4ConvRatio_text
                x: 20
                y: -630
                width: 133
                height: 20
                text: qsTr("Bank4 conv ratio")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 142
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Text {
                id: adcInt_text
                x: 24
                y: -626
                width: 133
                height: 20
                text: qsTr("ADC interval")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 168
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Text {
                id: advInt_text
                x: 24
                y: -600
                width: 133
                height: 20
                text: qsTr("Adv interval")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 29
                anchors.topMargin: 194
                font.pixelSize: 16
                font.styleName: "ExtraBold"
            }

            Button {
                id: button
                text: qsTr("Apply trim")
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 50
                anchors.topMargin: 10
                highlighted: true
                icon.color: "#0d0c0c"
                font.pointSize: 16
                onClicked: {
                    var buffer = new ArrayBuffer(21)
                    var view = new DataView(buffer)
                    var offset = 0

                    // uint8_t - mode
                    var mode = parseInt(modeInput.text, 10)
                    if (isNaN(mode)) return
                    view.setUint8(offset, mode)
                    offset += 1

                    // uint8_t - ADC error
                    var adcError = parseInt(adcError_input.text, 10)
                    if (isNaN(adcError)) return
                    view.setUint8(offset, adcError)
                    offset += 1

                    // uint32_t - bank1 conv ratio
                    var b1ConvRatio = parseInt(b1Ratio_input.text, 10)
                    if (isNaN(b1ConvRatio)) return
                    view.setUint32(offset, b1ConvRatio, true)
                    offset += 4

                    // uint32_t - bank2 conv ratio
                    var b2ConvRatio = parseInt(b2Ratio_input.text, 10)
                    if (isNaN(b2ConvRatio)) return
                    view.setUint32(offset, b2ConvRatio, true)
                    offset += 4

                    // uint32_t - bank3 conv ratio
                    var b3ConvRatio = parseInt(b3Ratio_input.text, 10)
                    if (isNaN(b3ConvRatio)) return
                    view.setUint32(offset, b3ConvRatio, true)
                    offset += 4

                    // uint32_t - bank4 conv ratio
                    var b4ConvRatio = parseInt(b4Ratio_input.text, 10)
                    if (isNaN(b4ConvRatio)) return
                    view.setUint32(offset, b4ConvRatio, true)
                    offset += 4

                    // uint8_t - ADC interval
                    var adcInt = parseInt(adcInt_input.text, 10)
                    if (isNaN(adcInt)) return
                    view.setUint8(offset, adcInt)
                    offset += 1

                    // uint16_t - Adv interval
                    var advInt = parseInt(advInt_input.text, 10)
                    if (isNaN(advInt)) return
                    view.setUint16(offset, advInt, true)

                    bleConnection.handleTrim(buffer)
                }
            }
            // BLE connection callbacks
            Connections {
                target: bleConnection
                ignoreUnknownSignals: true

                function onConnectedChanged() {
                    if (!bleConnection)
                        return

                    if (bleConnection.isConnected) {
                        console.log("BLE connected")
                    } else {
                        console.log("BLE disconnected")
                        modeInput.text = qsTr("?")
                        adcError_input.text = qsTr("?")
                        b1Ratio_input.text =  qsTr("?")
                        b2Ratio_input.text =  qsTr("?")
                        b3Ratio_input.text =  qsTr("?")
                        b4Ratio_input.text =  qsTr("?")
                        adcInt_input.text =  qsTr("?")
                        advInt_input.text =  qsTr("?")
                    }
                }
                // Handle received trim Value
                function onTrimReady() {
                    modeInput.text = bleConnection.trimMode.toString()
                    adcError_input.text = bleConnection.trimAdcError.toString()
                    b1Ratio_input.text =  bleConnection.b1ConvRatio.toString()
                    b2Ratio_input.text =  bleConnection.b2ConvRatio.toString()
                    b3Ratio_input.text =  bleConnection.b3ConvRatio.toString()
                    b4Ratio_input.text =  bleConnection.b4ConvRatio.toString()
                    adcInt_input.text =  bleConnection.adcInt
                    advInt_input.text =  bleConnection.advInt
                }
            }
        }
    }
}
