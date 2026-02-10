#include "BleConnection.h"
#include "BleConnectionWorker.h"

BleConnection::BleConnection(QObject *parent)
    : QObject(parent)
{
    setupWorker();
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
        emit connectedChanged();
        emit connected();
    });

    connect(m_worker, &BleConnectionWorker::disconnected, this, [this] {
        m_connected = false;
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
            this, &BleConnection::readCompleted);

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

//
// ---------- QML API ----------
//

void BleConnection::connectToDevice(QString address, QString name)
{
    QBluetoothAddress addr(address);
    QBluetoothDeviceInfo info(addr, name, 0);

    QMetaObject::invokeMethod(worker,
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

void BleConnection::read(const QBluetoothUuid &s,
                         const QBluetoothUuid &c)
{
    QMetaObject::invokeMethod(m_worker,
                              "read",
                              Qt::QueuedConnection,
                              Q_ARG(QBluetoothUuid, s),
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
