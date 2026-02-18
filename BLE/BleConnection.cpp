#include "BleConnection.h"
#include "BleConnectionWorker.h"

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

void BleConnection::on_readCompleted(QBluetoothUuid s, QBluetoothUuid c, QByteArray data)
{
    (void)s;

    quint16 uuid = 0;

    // --- Battery level (standard 16-bit) ---
    if (c.toUInt16() == 0x2A19) {
        uuid = 0x2A19;
    }
    // --- Your custom 128-bit UUID ---
    else if (c == QBluetoothUuid("37af9ae2-211d-4436-9d26-3a9ed02efeea")) {
        qDebug() << "[AIOS] sw_state_char read completed";
        uuid = 0x9AE2;   // your internal short ID
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
        }
        default:
        {
            break;
        }
    }
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
    } else {
        // 16-bit UUIDs
        ch = QBluetoothUuid(quint16(uuid));
    }
    qDebug() << "[read] " << "char: " << ch;

    read(ch);
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
