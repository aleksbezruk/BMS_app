#ifndef BLEMANAGER_H
#define BLEMANAGER_H

#include <QObject>
#include <QThread>

#include "blescannerworker.h"

// Inheriting from QObject is mandatory for the Qt Meta-Object system (Signals/Slots)
class BleManager : public QObject
{
    // The macro that enables all Qt magic (signals, slots, properties)
    Q_OBJECT

    // PROPERTY BINDING
    // This allows QML to "bind" to this variable.
    // In QML: `BusyIndicator { running: bleManager.scanning }`
    // If you change 'scanning' in C++, the UI updates automatically.
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
    explicit BleManager(QObject *parent = nullptr);
    ~BleManager();

    bool scanning() const { return scanning_; }

    // INVOKABLES
    // These are C++ functions that look like JavaScript functions to QML.
    // In QML: `Button { onClicked: bleManager.startScan() }`
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();

signals:
    // Emitted whenever 'scanning_' changes. Triggers the UI update.
    void scanningChanged();
    // Emitted when the worker finds a device.
    // In QML: `Connections { target: bleManager; function onDeviceFound(address, name) { ... } }`
    void deviceFound(QString address, QString name);


    // INTERNAL SIGNAL (Missing but needed!)
    // You likely need a signal here to talk to your Worker thread safely.
    // See the "Critical Implementation Detail" below.
    // void startWorkerScan();

private:
    bool scanning_ = false;

    QThread scannerThread_;
    BleScannerWorker *scanner_ = nullptr;
};

#endif // BLEMANAGER_H

/* [] END OF FILE */
