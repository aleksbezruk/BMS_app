#ifndef BLECONNECTION_CPP
#define BLECONNECTION_CPP

#include "BleConnection.h"
#include "BleConnectionWorker.h"

BleConnection::BleConnection(QObject *parent)
    : QObject(parent)
{
    worker = new BleConnectionWorker();
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished,
            worker, &QObject::deleteLater);

    // forward signals
    connect(worker, &BleConnectionWorker::connected,
            this, &BleConnection::connected);

    connect(worker, &BleConnectionWorker::disconnected,
            this, &BleConnection::disconnected);

    connect(worker, &BleConnectionWorker::servicesReady,
            this, &BleConnection::servicesReady);

    connect(worker, &BleConnectionWorker::error,
            this, &BleConnection::error);

    connect(worker, &BleConnectionWorker::notification,
            this, &BleConnection::notification);

    connect(worker, &BleConnectionWorker::readCompleted,
            this, &BleConnection::readCompleted);
}

BleConnection::~BleConnection()
{
    stop();
}

void BleConnection::start()
{
    workerThread.start();
}

void BleConnection::stop()
{
    workerThread.quit();
    workerThread.wait();
}

void BleConnection::connectToDevice(const QBluetoothDeviceInfo &info)
{
    QMetaObject::invokeMethod(worker,
                              "connectToDevice",
                              Qt::QueuedConnection,
                              Q_ARG(QBluetoothDeviceInfo, info));
}

void BleConnection::disconnectFromDevice()
{
    QMetaObject::invokeMethod(worker,
                              "disconnectFromDevice",
                              Qt::QueuedConnection);
}

void BleConnection::read(const QBluetoothUuid &s,
                         const QBluetoothUuid &c)
{
    QMetaObject::invokeMethod(worker,
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
    QMetaObject::invokeMethod(worker,
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
    QMetaObject::invokeMethod(worker,
                              "enableNotifications",
                              Qt::QueuedConnection,
                              Q_ARG(QBluetoothUuid, s),
                              Q_ARG(QBluetoothUuid, c));
}

#endif // BLECONNECTION_CPP
