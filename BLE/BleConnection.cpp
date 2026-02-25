#include "BleConnection.h"
#include "BleConnectionWorker.h"

static qint16 _calcVbat(QByteArray data);

BleConnection::BleConnection(QObject *parent)
    : QObject(parent)
{
    setupWorker();

    m_swState = 0;
}

BleConnection::~BleConnection()
{
    teardown();
}

void BleConnection::setupWorker()
{
    m_worker = new BleConnectionWorker();
    m_worker->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);

    // ---- worker → UI signals ----
    connect(m_worker, &BleConnectionWorker::connected, this, [this] {
        m_connected = true;
        qDebug() << "C++: connected signal received";
        emit connectedChanged();
        emit connected();
    });

    connect(m_worker, &BleConnectionWorker::disconnected, this, [this] {
        m_connected = false;
        qDebug() << "C++: disconnected signal received";
        emit connectedChanged();
        emit disconnected();

        // auto destroy when QML-created
        deleteLater();
    });

    connect(m_worker, &BleConnectionWorker::servicesReady,
            this, &BleConnection::servicesReady);

    connect(m_worker, &BleConnectionWorker::error,
            this, &BleConnection::error);

    connect(m_worker, &BleConnectionWorker::notification,
            this, &BleConnection::notification);

    connect(m_worker, &BleConnectionWorker::readCompleted,
            this, &BleConnection::on_readCompleted);

    connect(this, &BleConnection::servicesReady,
            this, &BleConnection::on_servicesReady);

    connect(this, &BleConnection::notification,
            this, &BleConnection::on_notification);

    m_thread.start();
}

void BleConnection::teardown()
{
    if (!m_thread.isRunning())
        return;

    QMetaObject::invokeMethod(m_worker,
                              "disconnectFromDevice",
                              Qt::QueuedConnection);

    m_thread.quit();
    m_thread.wait();
}

void BleConnection::on_servicesReady()
{
    // subscribe to BAS notifications (battery level in percent)
    qDebug() << "Subscribe to BAS";
    QBluetoothUuid bas_ch = QBluetoothUuid(quint16(0x2A19));
    QBluetoothUuid bas_svc = QBluetoothUuid(quint16(0x180F));
    enableNotifications(bas_svc, bas_ch);

    // subscribe to AIOS notifications (BMS swicthes state)
    qDebug() << "Subscribe to AIOS";
    QBluetoothUuid aios_ch = QBluetoothUuid("{37af9ae2-211d-4436-9d26-3a9ed02efeea}");
    QBluetoothUuid aios_svc = QBluetoothUuid(quint16(0x1815));
    enableNotifications(aios_svc, aios_ch);
}

void BleConnection::on_readCompleted(QBluetoothUuid s, QBluetoothUuid c, QByteArray data)
{
    (void)s;

    quint16 uuid = 0;

    qint16 vbat = 0;
    qint16 vbank1 = 0;
    qint16 vbank2 = 0;
    qint16 vbank3 = 0;
    qint16 vbank4 = 0;

    // --- Battery level (standard 16-bit) ---
    if (c.toUInt16() == 0x2A19) {
        uuid = 0x2A19;
    }
    // --- Your custom 128-bit UUID ---
    else if (c == QBluetoothUuid("37af9ae2-211d-4436-9d26-3a9ed02efeea")) {
        qDebug() << "[AIOS] sw_state_char read completed";
        uuid = 0x9AE2;   // your internal short ID
    } else if (c == QBluetoothUuid("170ad8db-5244-4926-963e-417099122bba")) {
        vbat = _calcVbat(data);
        // qDebug() << "[AIOS] full_VBAT raw data: " <<  static_cast<quint16>(data[0]) << ", " << static_cast<quint16>(data[1]);
        qDebug() << "[AIOS] full_VBAT read completed: " << vbat;
        uuid = 0x2BBA;   // your internal short ID
    } else if (c == QBluetoothUuid("170ad8db-5244-4926-963e-417099122bb1")) {
        vbank1 = _calcVbat(data);
        qDebug() << "[AIOS] bank1 read completed: " << vbank1;
        uuid = 0x2BB1;   // your internal short ID
    } else if (c == QBluetoothUuid("170ad8db-5244-4926-963e-417099122bb2")) {
        vbank2 = _calcVbat(data);
        qDebug() << "[AIOS] bank2 read completed: " << vbank2;
        uuid = 0x2BB2;   // your internal short ID
    } else if (c == QBluetoothUuid("170ad8db-5244-4926-963e-417099122bb3")) {
        vbank3 = _calcVbat(data);
        qDebug() << "[AIOS] bank3 read completed: " << vbank3;
        uuid = 0x2BB3;   // your internal short ID
    } else if (c == QBluetoothUuid("170ad8db-5244-4926-963e-417099122bb4")) {
        vbank4 = _calcVbat(data);
        qDebug() << "[AIOS] bank4 read completed: " << vbank4;
        uuid = 0x2BB4;   // your internal short ID
    }
    // --- fallback for other 16-bit UUIDs ---
    else if (c.toUInt16()) {
        uuid = c.toUInt16();
    }

    switch (uuid) {
        case 0x2A19:
        {
            updateBattery(static_cast<quint8>(data[0]));
            break;
        }
        case 0x9AE2:
        {
            qDebug() << "[AIOS], " << "SW state: " << static_cast<quint8>(data[0]);
            setSwState(static_cast<quint8>(data[0]));
            break;
        }
        case 0x2BBA:
        {
            m_fullVbat = vbat;
            emit fullVbatChanged();
            break;
        }
        case 0x2BB1:
        {
            m_bank1Volt = vbank1;
            emit bank1VoltChanged();
            break;
        }
        case 0x2BB2:
        {
            m_bank2Volt = vbank2;
            emit bank2VoltChanged();
            break;
        }
        case 0x2BB3:
        {
            m_bank3Volt = vbank3;
            emit bank3VoltChanged();
            break;
        }
        case 0x2BB4:
        {
            m_bank4Volt = vbank4;
            emit bank4VoltChanged();
            break;
        }
        default:
        {
            break;
        }
    }
}

static qint16 _calcVbat(QByteArray data)
{
    qint16 vbat = 0;

    quint8 lsb = static_cast<quint8>(data[0]);
    quint8 msb = static_cast<quint8>(data[1]);

    vbat = (((quint16) lsb) & 0x00FF) | (((quint16) msb) <<8);

    return vbat;
}

//
// ---------- QML API ----------
//

void BleConnection::connectToDevice(QString address, QString name)
{
    QBluetoothAddress addr(address);
    QBluetoothDeviceInfo info(addr, name, 0);

    info.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);

    QMetaObject::invokeMethod(m_worker,
                              "connectToDevice",
                              Qt::QueuedConnection,
                              Q_ARG(QBluetoothDeviceInfo, info));
}

void BleConnection::disconnectDevice()
{
    QMetaObject::invokeMethod(m_worker,
                              "disconnectFromDevice",
                              Qt::QueuedConnection);
}

void BleConnection::readChar(unsigned int uuid)
{
    QBluetoothUuid ch;
    if (uuid == 0x9AE2) {
        // 128-bit UUIDs
        ch = QBluetoothUuid("{37af9ae2-211d-4436-9d26-3a9ed02efeea}");
    } else if (uuid == 0x2BBA) {
        // Full VBAT in mV
        ch = QBluetoothUuid("{170ad8db-5244-4926-963e-417099122bba}");
    } else if (uuid == 0x2BB1) {
        // Bank1 in mV
        ch = QBluetoothUuid("{170ad8db-5244-4926-963e-417099122bb1}");
    } else if (uuid == 0x2BB2) {
        // Bank2 in mV
        ch = QBluetoothUuid("{170ad8db-5244-4926-963e-417099122bb2}");
    } else if (uuid == 0x2BB3) {
        // Bank3 in mV
        ch = QBluetoothUuid("{170ad8db-5244-4926-963e-417099122bb3}");
    } else if (uuid == 0x2BB4) {
        // Bank4 in mV
        ch = QBluetoothUuid("{170ad8db-5244-4926-963e-417099122bb4}");
    } else {
        // 16-bit UUIDs
        ch = QBluetoothUuid(quint16(uuid));
    }
    qDebug() << "[read] " << "char: " << ch;

    read(ch);

    // add some delay between reads to avoid stack overloading
    // QThread::msleep(500);
}

void BleConnection::toggleSwitch(quint8 mask)
{
    m_swState = m_swState ^ mask;   // toggle
    QByteArray data;
    data.append(static_cast<char>(m_swState));

    QBluetoothUuid ch, svc;
    ch = QBluetoothUuid("{37af9ae2-211d-4436-9d26-3a9ed02efeea}");
    svc = QBluetoothUuid(quint16(0x1815));

    write(svc, ch, data, true); // with write response
}

 void BleConnection::on_notification(QBluetoothUuid service, QBluetoothUuid characteristic, QByteArray data)
{
     qDebug() << "Notification received, " << "characteristic: " << characteristic << " data=" << static_cast<quint8>(data[0]);

    // --- Battery level (standard 16-bit) ---
    if (characteristic.toUInt16() == 0x2A19) {
        updateBattery(static_cast<quint8>(data[0]));
    }
    // --- AIOS custom 128-bit UUID ---
    else if (characteristic == QBluetoothUuid("37af9ae2-211d-4436-9d26-3a9ed02efeea")) {
        setSwState(static_cast<quint8>(data[0]));
    }
}

void BleConnection::read(const QBluetoothUuid &c)
{
    QMetaObject::invokeMethod(m_worker,
                              "read",
                              Qt::QueuedConnection,
                              Q_ARG(QBluetoothUuid, c));
}

void BleConnection::write(const QBluetoothUuid &s,
                          const QBluetoothUuid &c,
                          const QByteArray &d,
                          bool resp)
{
    QMetaObject::invokeMethod(m_worker,
                              "write",
                              Qt::QueuedConnection,
                              Q_ARG(QBluetoothUuid, s),
                              Q_ARG(QBluetoothUuid, c),
                              Q_ARG(QByteArray, d),
                              Q_ARG(bool, resp));
}

void BleConnection::enableNotifications(const QBluetoothUuid &s,
                                        const QBluetoothUuid &c)
{
    QMetaObject::invokeMethod(m_worker,
                              "enableNotifications",
                              Qt::QueuedConnection,
                              Q_ARG(QBluetoothUuid, s),
                              Q_ARG(QBluetoothUuid, c));
}

int BleConnection::batteryLevel() const
{
    return m_batteryLevel;
}

quint8 BleConnection::swState() const
{
    return m_swState;
}

void BleConnection::setSwState(quint8 newSwState)
{
    if (m_swState == newSwState)
        return;
    m_swState = newSwState;
    emit swStateChanged();
}

qint16 BleConnection::fullVbat() const
{
    return m_fullVbat;
}

qint16 BleConnection::bank1Volt() const
{
    return m_bank1Volt;
}

qint16 BleConnection::bank2Volt() const
{
    return m_bank2Volt;
}

qint16 BleConnection::bank3Volt() const
{
    return m_bank3Volt;
}

qint16 BleConnection::bank4Volt() const
{
    return m_bank4Volt;
}
