cmake_minimum_required(VERSION 3.16)

project(BMS_PCBAtestApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Bluetooth
)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appBMS_PCBAtestApp
    main.cpp
)

target_compile_definitions(appBMS_PCBAtestApp
    PRIVATE
        USE_QT_BLUETOOTH
        BMS_TEST_APP
)

target_include_directories(appBMS_PCBAtestApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../BLE
)

qt_add_qml_module(appBMS_PCBAtestApp
    URI BMS_PCBAtestApp
    QML_FILES
        Main.qml
    SOURCES
        ../BLE/BleConnection.cpp
        ../BLE/BleConnection.h
        ../BLE/BleConnectionWorker.cpp
        ../BLE/BleConnectionWorker.h
        ../BLE/bledevice.h
        ../BLE/blemanager.cpp
        ../BLE/blemanager.h
        ../BLE/blescannerworker.cpp
        ../BLE/blescannerworker.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appBMS_PCBAtestApp PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appBMS_PCBAtestApp
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appBMS_PCBAtestApp PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
    Qt6::Bluetooth
    Threads::Threads
)

include(GNUInstallDirs)
install(TARGETS appBMS_PCBAtestApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
