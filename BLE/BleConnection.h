#pragma once

#include <QObject>
#include <QThread>
#include <QtBluetooth>

#include <QtQml/qqmlregistration.h>

class BleConnectionWorker;

class BleConnection : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectedChanged)

    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)

    Q_PROPERTY(quint8 swState READ swState WRITE setSwState NOTIFY swStateChanged)

    Q_PROPERTY(qint16 fullVbat READ fullVbat NOTIFY fullVbatChanged)

    Q_PROPERTY(qint16 bank1Volt READ bank1Volt NOTIFY bank1VoltChanged)

    Q_PROPERTY(qint16 bank2Volt READ bank2Volt NOTIFY bank2VoltChanged)

    Q_PROPERTY(qint16 bank3Volt READ bank3Volt NOTIFY bank3VoltChanged)

    Q_PROPERTY(qint16 bank4Volt READ bank4Volt NOTIFY bank4VoltChanged)

public:
    explicit BleConnection(QObject *parent = nullptr);
    ~BleConnection();

    bool isConnected() const { return m_connected; }

    // ---- QML API ----
    Q_INVOKABLE void connectToDevice(QString address, QString name);
    Q_INVOKABLE void disconnectDevice();

    Q_INVOKABLE void readChar(unsigned int uuid);

    Q_INVOKABLE void write(const QBluetoothUuid &service,
                           const QBluetoothUuid &characteristic,
                           const QByteArray &data,
                           bool withResponse = true);

    Q_INVOKABLE void enableNotifications(const QBluetoothUuid &service,
                                         const QBluetoothUuid &characteristic);

    Q_INVOKABLE void toggleSwitch(quint8 mask);

    int batteryLevel() const;

    quint8 swState() const;
    void setSwState(quint8 newSwState);

    qint16 fullVbat() const;

    qint16 bank1Volt() const;

    qint16 bank2Volt() const;

    qint16 bank3Volt() const;

    qint16 bank4Volt() const;

signals:
    void connected();
    void disconnected();
    void connectedChanged();
    void servicesReady();
    void error(QString);

    void notification(QBluetoothUuid service,
                      QBluetoothUuid characteristic,
                      QByteArray data);

    void batteryLevelChanged();

    void swStateChanged();

    void fullVbatChanged();

    void bank1VoltChanged();

    void bank2VoltChanged();

    void bank3VoltChanged();

    void bank4VoltChanged();

public slots:
    void on_readCompleted(QBluetoothUuid service,
                          QBluetoothUuid characteristic,
                          QByteArray data);
    void updateBattery(quint8 value)
    {
        if (m_batteryLevel == value)
            return;

        m_batteryLevel = value;
        emit batteryLevelChanged();
    }
    void on_servicesReady();
    void on_notification(QBluetoothUuid service, QBluetoothUuid characteristic, QByteArray data);

private:
    QThread m_thread;
    BleConnectionWorker *m_worker = nullptr;
    bool m_connected = false;

    void setupWorker();
    void teardown();

    void read(const QBluetoothUuid &characteristic);
    int m_batteryLevel;
    quint8 m_swState;
    qint16 m_fullVbat;
    qint16 m_bank1Volt;
    qint16 m_bank2Volt;
    qint16 m_bank3Volt;
    qint16 m_bank4Volt;
};
