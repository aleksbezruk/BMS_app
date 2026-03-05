# Battery Mnagement System (BMS) app on BLE

---
## Features

1. BLE scan for BMS device (peripheral)
2. Connect to BMS device
3. Discover BMS device attributes
4. Subscribe to BAS notifications
5. Subscribe to AIOS switch state notifications
6. Read switches state
7. Read: full battery level [mV]
8. Read separate banks level [mV]

## Design, implementation
Built with Qt Framework:
- UI written in QML;
- BLE backen written in C++;
- QtBluetooth library is used for BLE communication.
- tested on: Ubuntu desktop 24.04 and Android 12.

## References
BMS firmware repo: https://github.com/aleksbezruk/BMS_onMCU_BLE.git
