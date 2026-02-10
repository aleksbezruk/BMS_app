// 1. Core Qt GUI module. Needed for apps with a GUI but without QWidgets (perfect for QML).
#include <QGuiApplication>

// 2. The engine that loads QML files and manages the QML environment
#include <QQmlApplicationEngine>

// 3. Required to access the "root context" to inject C++ objects into QML.
#include <QQmlContext>

#include "BLE/blemanager.h"

#include "BLE/BleConnection.h"

#include <QPermissions>

int main(int argc, char *argv[])
{
    // Initialize the application. This handles system-specific setup
    // (like high-DPI scaling, parsing command line args).
    QGuiApplication app(argc, argv);

    qmlRegisterType<BleConnection>("BMS.BLE", 1, 0, "BleConnection");

    qRegisterMetaType<QBluetoothDeviceInfo>();

#ifdef Q_OS_ANDROID
    // Runtime permission
    QBluetoothPermission perm;
    perm.setCommunicationModes(QBluetoothPermission::Access);

    app.requestPermission(perm, [](const QPermission &p){
        qDebug() << "BLE permission:"
                 << (p.status() == Qt::PermissionStatus::Granted);
    });
#endif

    // Create the QML engine. This is the interpreter for your UI.
    QQmlApplicationEngine engine;

    // Instantiate app specific (BMS) C++ Bluetooth Manager class on the stack.
    BleManager bleManager;

    // THE BRIDGE: This exposes the C++ object 'bleManager' to QML.
    // In QML, you can now refer to it simply as `bleManager`.
    // Example in QML: `bleManager.startScan()
    engine.rootContext()->setContextProperty(QStringLiteral("bleManager"),
                                             &bleManager);

    // A safety check.
    // Connects the engine's "objectCreationFailed" signal to a lambda function.
    // If QML fails to load (e.g., syntax error), the app exits with error code -1.
    // Without this, the app might run as a "zombie" process with no UI.
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Loads the "Main" QML file from your module "BMS_app".
    // This is the specific syntax for Qt 6 using CMake API.
    engine.loadFromModule("BMS", "Main");

    // Enters the main (GUI) event loop.
    // The app waits here, processing user clicks, BLE signals, and GUI updates
    // until the window is closed.
    return app.exec();
}
