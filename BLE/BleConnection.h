#pragma once

#include <QObject>
#include <QThread>
#include <QtBluetooth>

class BleConnectionWorker;

class BleConnection : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit BleConnection(QObject *parent = nullptr);
    ~BleConnection();

    bool isConnected() const { return m_connected; }

    // ---- QML API ----
    Q_INVOKABLE void connectToDevice(QString address, QString name);
    Q_INVOKABLE void disconnectDevice();

    Q_INVOKABLE void read(const QBluetoothUuid &service,
                          const QBluetoothUuid &characteristic);

    Q_INVOKABLE void write(const QBluetoothUuid &service,
                           const QBluetoothUuid &characteristic,
                           const QByteArray &data,
                           bool withResponse = true);

    Q_INVOKABLE void enableNotifications(const QBluetoothUuid &service,
                                         const QBluetoothUuid &characteristic);

signals:
    void connected();
    void disconnected();
    void connectedChanged();
    void servicesReady();
    void error(QString);

    void notification(QBluetoothUuid service,
                      QBluetoothUuid characteristic,
                      QByteArray data);

    void readCompleted(QBluetoothUuid service,
                       QBluetoothUuid characteristic,
                       QByteArray data);

private:
    QThread m_thread;
    BleConnectionWorker *m_worker = nullptr;
    bool m_connected = false;

    void setupWorker();
    void teardown();
};
