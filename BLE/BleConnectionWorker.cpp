#include "BleConnectionWorker.h"
#include <QDebug>

BleConnectionWorker::BleConnectionWorker(QObject *parent)
    : QObject(parent)
{
}

BleConnectionWorker::~BleConnectionWorker()
{
    if (controller)
        controller->disconnectFromDevice();
}

void BleConnectionWorker::connectToDevice(const QBluetoothDeviceInfo &info)
{
    deviceInfo = info;

    controller = QLowEnergyController::createCentral(deviceInfo, this);

    connect(controller, &QLowEnergyController::connected,
            this, &BleConnectionWorker::onConnected);

    connect(controller, &QLowEnergyController::disconnected,
            this, &BleConnectionWorker::onDisconnected);

    connect(controller, &QLowEnergyController::errorOccurred,
            this, &BleConnectionWorker::onError);

    connect(controller, &QLowEnergyController::discoveryFinished,
            this, &BleConnectionWorker::onServiceScanDone);

    controller->connectToDevice();
}

void BleConnectionWorker::disconnectFromDevice()
{
    if (controller)
        controller->disconnectFromDevice();
}

void BleConnectionWorker::onConnected()
{
    emit connected();
    controller->discoverServices();

    qDebug() << "Start discovery of services";
}

void BleConnectionWorker::onDisconnected()
{
    emit disconnected();

    for (auto s : services)
        s->deleteLater();

    services.clear();
    charToService.clear();
}

void BleConnectionWorker::onError(QLowEnergyController::Error e)
{
    emit error(QString("BLE error: %1").arg(e));
}

void BleConnectionWorker::onServiceScanDone()
{
    for (auto uuid : controller->services()) {

        auto service = controller->createServiceObject(uuid, this);
        if (!service)
            continue;

        qDebug() << "service uuid: " << uuid.toString();
        services.insert(uuid, service);

        connect(service, &QLowEnergyService::stateChanged,
                this, &BleConnectionWorker::onServiceStateChanged);

        connect(service, &QLowEnergyService::characteristicChanged,
                this, &BleConnectionWorker::onCharacteristicChanged);

        connect(service, &QLowEnergyService::characteristicRead,
                this, &BleConnectionWorker::onCharacteristicRead);

        connect(service, &QLowEnergyService::characteristicWritten,
                this, &BleConnectionWorker::onCharacteristicWritten);

        service->discoverDetails();
    }
}

void BleConnectionWorker::onServiceStateChanged(QLowEnergyService::ServiceState)
{
    bool ready = true;

    for (auto s : services)
        if (s->state() != QLowEnergyService::RemoteServiceDiscovered)
            ready = false;

    if (!ready)
        return;

    // build char → service map
    for (auto serviceUuid : services.keys()) {
        auto service = services.value(serviceUuid);

        for (auto c : service->characteristics()) {
            qDebug() << "[on_svc_chngd] " << "svc: " << serviceUuid << " char: " << c.uuid();
            charToService.insert(c.uuid(), serviceUuid);
        }

    }

    emit servicesReady();
}

QLowEnergyService* BleConnectionWorker::getService(const QBluetoothUuid &uuid) const
{
    return services.value(uuid, nullptr);
}

void BleConnectionWorker::read(const QBluetoothUuid &c)
{
    QBluetoothUuid s = charToService.value(c);

    auto srv = getService(s);
    if (!srv) {
        qDebug() << "[read_char]: not valid svc";
        return;
    }

    auto ch = srv->characteristic(c);
    if (!ch.isValid()) {
        qDebug() << "[read_char]: not valid char";
        return;
    }

    srv->readCharacteristic(ch);
}

void BleConnectionWorker::write(const QBluetoothUuid &s,
                                const QBluetoothUuid &c,
                                const QByteArray &data,
                                bool resp)
{
    auto srv = getService(s);
    if (!srv)
        return;

    auto ch = srv->characteristic(c);
    if (!ch.isValid())
        return;

    writeQueue.enqueue({srv, ch, data, resp});
    processWriteQueue();
}

void BleConnectionWorker::processWriteQueue()
{
    if (writeInProgress || writeQueue.isEmpty())
        return;

    auto r = writeQueue.dequeue();
    writeInProgress = true;

    auto mode = r.withResponse ?
                    QLowEnergyService::WriteWithResponse :
                    QLowEnergyService::WriteWithoutResponse;

    qDebug() << "Write char: " << r.characteristic.uuid().toUInt128() << " data = " << r.data;
    r.service->writeCharacteristic(r.characteristic, r.data, mode);

    if (!r.withResponse) {
        writeInProgress = false;
        QMetaObject::invokeMethod(this,
                                  &BleConnectionWorker::processWriteQueue,
                                  Qt::QueuedConnection);
    }
}

void BleConnectionWorker::enableNotifications(const QBluetoothUuid &s,
                                              const QBluetoothUuid &c)
{
    auto srv = getService(s);
    if (!srv)
        return;

    auto ch = srv->characteristic(c);
    if (!ch.isValid())
        return;

    auto desc = ch.descriptor(
        QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

    if (desc.isValid())
        srv->writeDescriptor(desc, QByteArray::fromHex("0100"));
}

void BleConnectionWorker::onCharacteristicChanged(const QLowEnergyCharacteristic &c,
                                                  const QByteArray &value)
{
    auto serviceUuid = charToService.value(c.uuid());
    emit notification(serviceUuid, c.uuid(), value);
}

void BleConnectionWorker::onCharacteristicRead(const QLowEnergyCharacteristic &c,
                                               const QByteArray &value)
{
    auto serviceUuid = charToService.value(c.uuid());

    qDebug() << "[on char_read]" << "char: " << c.uuid() << "val = " << static_cast<quint8>(value[0]);

    emit readCompleted(serviceUuid, c.uuid(), value);
}

void BleConnectionWorker::onCharacteristicWritten(const QLowEnergyCharacteristic &,
                                                  const QByteArray &)
{
    writeInProgress = false;
    processWriteQueue();
}
