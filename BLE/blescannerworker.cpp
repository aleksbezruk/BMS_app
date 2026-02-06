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
}

void BleScannerWorker::initAdapter()
{
    // TODO: impplement for QtBluetooth
    this->adapter_ = NULL;
}

void BleScannerWorker::startScan()
{
#ifdef Q_OS_WIN
    // MUST be called inside the worker thread
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
#endif

    // open BLE adapter
    _openAdapter(this);

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

    // stop scan on BLE adapter
    _stopScan(this);
}

BleScannerWorker::~BleScannerWorker()
{
    stopScan();

    // Give WinRT time to drain callbacks
    QThread::msleep(50);
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
    // TODO: impplement for QtBluetooth
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
    // TODO: implement for QtBluetooth
#endif
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
    // TODO: implement for QtBluetooth
#endif
}

/* [] END OF FILE */
