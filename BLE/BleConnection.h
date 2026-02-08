#ifndef BLECONNECTION_H
#define BLECONNECTION_H

#include <QObject>
#include <QtBluetooth>
#include <QThread>

class BleConnectionWorker;

class BleConnection : public QObject
{
    Q_OBJECT
public:
    explicit BleConnection(QObject *parent = nullptr);
    ~BleConnection();

    void start();
    void stop();

    void connectToDevice(const QBluetoothDeviceInfo &info);
    void disconnectFromDevice();

    void read(const QBluetoothUuid &service,
              const QBluetoothUuid &characteristic);

    void write(const QBluetoothUuid &service,
               const QBluetoothUuid &characteristic,
               const QByteArray &data,
               bool withResponse = true);

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

private:
    QThread workerThread;
    BleConnectionWorker *worker = nullptr;
};

#endif // BLECONNECTION_H
