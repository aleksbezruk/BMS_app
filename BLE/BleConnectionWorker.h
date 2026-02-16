#ifndef BLECONNECTIONWORKER_H
#define BLECONNECTIONWORKER_H

#include <QObject>
#include <QtBluetooth>
#include <QQueue>
#include <QHash>

class BleConnectionWorker : public QObject
{
    Q_OBJECT
public:
    explicit BleConnectionWorker(QObject *parent = nullptr);
    ~BleConnectionWorker();

public slots:
    void connectToDevice(const QBluetoothDeviceInfo &info);
    void disconnectFromDevice();

    void read(const QBluetoothUuid &characteristic);

    void write(const QBluetoothUuid &service,
               const QBluetoothUuid &characteristic,
               const QByteArray &data,
               bool withResponse);

    void enableNotifications(const QBluetoothUuid &service,
                             const QBluetoothUuid &characteristic);

signals:
    void connected();
    void disconnected();
    void servicesReady();
    void error(QString);

    void notification(QBluetoothUuid service,
                      QBluetoothUuid characteristic,
                      QByteArray data);

    void readCompleted(QBluetoothUuid service,
                       QBluetoothUuid characteristic,
                       QByteArray data);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QLowEnergyController::Error);
    void onServiceScanDone();

    void onServiceStateChanged(QLowEnergyService::ServiceState s);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &c,
                                 const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &c,
                                 const QByteArray &value);

private:
    struct WriteRequest {
        QLowEnergyService *service;
        QLowEnergyCharacteristic characteristic;
        QByteArray data;
        bool withResponse;
    };

    void processWriteQueue();
    QLowEnergyService* getService(const QBluetoothUuid &uuid) const;

private:
    QBluetoothDeviceInfo deviceInfo;
    QLowEnergyController *controller = nullptr;

    QHash<QBluetoothUuid, QLowEnergyService*> services;

    QHash<QBluetoothUuid, QBluetoothUuid> charToService;

    QQueue<WriteRequest> writeQueue;
    bool writeInProgress = false;
};

#endif
