#include "blescannerworker.h"
#include <QDebug>
#include <QThread>

#ifdef Q_OS_WIN
#include <winrt/base.h>
#endif

#ifdef Q_OS_WIN
#include <objbase.h> // Required for COM initialization
#endif

static void _openAdapter(BleScannerWorker* scanner);
static void _setScanCallback(BleScannerWorker* scanner);
static void _startScan(BleScannerWorker* scanner);
static void _stopScan(BleScannerWorker* scanner);

BleScannerWorker::BleScannerWorker(QObject *parent)
    : QObject(parent)
{
    // open BLE adapter
    _openAdapter(this);
}

void BleScannerWorker::initAdapter()
{
    this->adapter_ = new QBluetoothDeviceDiscoveryAgent(this);
    this->adapter_->setLowEnergyDiscoveryTimeout(15000);

    connect(this->adapter_,
            &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this,
            &BleScannerWorker::deviceDiscovered);

    connect(this->adapter_,
            &QBluetoothDeviceDiscoveryAgent::finished,
            this,
            &BleScannerWorker::scanFinished);

    connect(this->adapter_,
            &QBluetoothDeviceDiscoveryAgent::canceled,
            this,
            &BleScannerWorker::onStopped);
}

void BleScannerWorker::startScan()
{
#ifdef Q_OS_WIN
    // MUST be called inside the worker thread
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
#endif

    stopRequested_ = false;
    scanning_ = true;
    emit scanStarted();

    // setup scan callabck
    _setScanCallback(this);

    // start scanning
    _startScan(this);

}

void BleScannerWorker::stopScan()
{
    if (!scanning_) return;

    scanning_ = false;      // 🔒 FIRST
    stopRequested_ = true;
    emit scanStopped();

    // stop scan on BLE adapter
    _stopScan(this);
}

BleScannerWorker::~BleScannerWorker()
{
    stopScan();

    // Give WinRT time to drain callbacks
    QThread::msleep(50);
}

void BleScannerWorker::onStopped()
{
    qDebug() << "Scan stopped manually";
}

void BleScannerWorker::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (!(info.coreConfigurations()
          & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
    {
        return;
    }

    QString name = info.name().isEmpty()
                       ? "Unknown"
                       : info.name();
    QString addr = info.address().toString();

    emit this->deviceFound(
        addr,
        name
    );
}

void BleScannerWorker::scanFinished()
{
    emit scanStopped();
    qDebug() << "Scan finished";
}

static void _stopScan(BleScannerWorker* scanner)
{
#if defined(USE_SIMPLEBLE)
    if (scanner.adapter_) {
        try {
            scanner->adapter_->set_callback_on_scan_found(nullptr); // 🔥 MOST IMPORTANT
            scanner->adapter_->scan_stop();
        } catch (...) {
        }
    }

    emit scanner->scanStopped();
#else
    if (scanner->isActive())
        scanner->adapterStopScan();
#endif
}

bool BleScannerWorker::isActive()
{
    return adapter_->isActive();
}
void BleScannerWorker::adapterStopScan()
{
    adapter_->stop();
}
void BleScannerWorker::adapterStartScan()
{
    adapter_->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

// ====================
// Auxiliary functions
// ====================
static void _openAdapter(BleScannerWorker* scanner)
{
#if defined(USE_SIMPLEBLE)
    if (!scanner->adapter_) {
        auto adapters = SimpleBLE::Adapter::get_adapters();
        if (adapters.empty()) {
            emit scanner->scanStopped();
            return;
        }
        scanner->adapter_ = std::make_unique<SimpleBLE::Adapter>(adapters.front());
    }
#else
    scanner->initAdapter();
#endif
}

static void _setScanCallback(BleScannerWorker* scanner)
{
#if defined(USE_SIMPLEBLE)
    scanner->adapter_->set_callback_on_scan_found(
        [scanner](SimpleBLE::Peripheral peripheral)
        {
            // HARD EXIT — WinRT-safe
            if (!scanner->scanning_.load(std::memory_order_acquire))
                return;

            std::string addr;
            std::string name;

            try {
                addr = peripheral.address();
                name = peripheral.identifier();
            } catch (...) {
                return; // WinRT already tearing down
            }

            QMetaObject::invokeMethod(
                scanner,
                [scanner, addr = std::move(addr), name = std::move(name)]()
                {
                    if (!scanner->scanning_) return;
                    emit scanner->deviceFound(
                        QString::fromStdString(addr),
                        QString::fromStdString(name)
                        );
                },
                Qt::QueuedConnection
                );
        });
#else
    // Fall through for QtBluetooth
#endif
}

static void _startScan(BleScannerWorker* scanner)
{
#if defined(USE_SIMPLEBLE)
    try {
        qDebug() << "Calling scan_start()...";
        scanner->adapter_->scan_start();
    } catch (...) {
        scanner->scanning_ = false;
        emit scanner.scanStopped();
    }
#else
    scanner->adapterStartScan();
    qDebug() << "Scan started";
#endif
}

/* [] END OF FILE */
