cmake_minimum_required(VERSION 3.16)

project(BMS VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(USE_SIMPLEBLE "Use SimpleBLE backend (Desktop only)" OFF)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
)

qt_policy(SET QTP0001 NEW)

if (NOT USE_SIMPLEBLE)
    find_package(Qt6 REQUIRED COMPONENTS Bluetooth)
endif()

find_package(Threads REQUIRED)

# -------------------------
# APP TARGET
# -------------------------
qt_add_executable(BMS_app
    main.cpp
    BLE/blemanager.cpp
)

qt_add_qml_module(BMS_app
    URI BMS
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        BLE/blemanager.h
        BLE/blescannerworker.cpp
        BLE/blescannerworker.h
        BLE/BleConnection.h
        BLE/BleConnectionWorker.h
        BLE/BleConnection.cpp
        BLE/BleConnectionWorker.cpp
        RESOURCES images/battery.png
        RESOURCES README.md
        RESOURCES images/battery_yellow.png
        images/battery_red.png
)

target_include_directories(BMS_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/BLE
)

# -------------------------
# BACKEND
# -------------------------
if (USE_SIMPLEBLE AND NOT ANDROID)
    message(STATUS "Using SimpleBLE backend")

    add_subdirectory(
        ../../simpleble/simpleble
        ${CMAKE_BINARY_DIR}/simpleble
    )

    target_compile_definitions(BMS_app PRIVATE USE_SIMPLEBLE)

    target_link_libraries(BMS_app PRIVATE
        Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
        simpleble
        Threads::Threads
    )

    if (UNIX)
        target_link_libraries(BMS_app PRIVATE dl)
    endif()

else()
    message(STATUS "Using QtBluetooth backend")

    target_compile_definitions(BMS_app PRIVATE USE_QT_BLUETOOTH)

    target_link_libraries(BMS_app PRIVATE
        Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
        Qt6::Bluetooth
        Threads::Threads
    )
endif()

# -------------------------
# INSTALL (desktop)
# -------------------------
if (NOT ANDROID)
    include(GNUInstallDirs)
    install(TARGETS BMS_app
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
