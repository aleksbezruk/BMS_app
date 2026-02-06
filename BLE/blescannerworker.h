#ifndef BLESCANNERWORKER_H
#define BLESCANNERWORKER_H

#include <QObject>
#include <atomic>

#if defined(USE_SIMPLEBLE)
#include <simpleble/SimpleBLE.h>
#else
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#endif

class BleScannerWorker : public QObject
{
    Q_OBJECT
public:
    explicit BleScannerWorker(QObject *parent = nullptr);
    ~BleScannerWorker();
    void initAdapter();
    bool isActive();
    void adapterStopScan();
    void adapterStartScan();

public slots:
    void startScan();
    void stopScan();
    // Handlers for QtBluetooth events
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void scanFinished();
    void onStopped();

signals:
    void deviceFound(QString address, QString name);
    void scanStarted();
    void scanStopped();

private:
#if defined(USE_SIMPLEBLE)
    std::unique_ptr<SimpleBLE::Adapter> adapter_;
#else
    QBluetoothDeviceDiscoveryAgent* adapter_;
#endif
    bool adapterInitialized_ = false; // Fixes: 'Use of undeclared identifier'
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> scanning_{false};
};

#endif // BLESCANNERWORKER_H

/* [] END OF FILE */
