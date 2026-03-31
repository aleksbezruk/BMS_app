#include <QGuiApplication>
#include <QQmlApplicationEngine>

// Required to access the "root context" to inject C++ objects into QML.
#include <QQmlContext>

#include "blemanager.h"

#include "BleConnection.h"

#include <QPermissions>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

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

    // Instantiate app specific (BMS) C++ Bluetooth Manager class on the stack.
    BleManager bleManager;

    // Create the QML engine. This is the interpreter for your UI.
    QQmlApplicationEngine engine;

    // THE BRIDGE: This exposes the C++ object 'bleManager' to QML.
    // In QML, you can now refer to it simply as `bleManager`.
    // Example in QML: `bleManager.startScan()
    engine.rootContext()->setContextProperty(QStringLiteral("bleManager"),
                                             &bleManager);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("BMS_PCBAtestApp", "Main");

    return app.exec();
}
