/**
 * @file blemanager.cpp
 * @details
 * 1. QMetaObject::invokeMethod vs. Signals
 *    It's possible to create a signal startWorkerScan to comminicate between Main Thread and Scan THreda.
 *    Chosen QMetaObject::invokeMethod approach.
 *    The approach: invokeMethod dynamically looks up a function by name ("startScan") and calls it.
 *    Qt::QueuedConnection forces it to run in the scannerThread_'s event loop.
 */

#include "blemanager.h"
#include <QDebug>

BleManager::BleManager(QObject *parent)
    : QObject(parent)
{
    scanner_ = new BleScannerWorker();
    scanner_->moveToThread(&scannerThread_);

    // Auto-delete worker (scanner_) when thread finishes
    connect(&scannerThread_, &QThread::finished,
            scanner_, &QObject::deleteLater);

    // Stop thread when manager is destroyed
    connect(this, &BleManager::destroyed,
            &scannerThread_, &QThread::quit);

    // React to scan start
    connect(scanner_, &BleScannerWorker::scanStarted, this, [this]() {
        scanning_ = true;
        emit scanningChanged();
    });

    connect(scanner_, &BleScannerWorker::scanStopped, this, [this]() {
        scanning_ = false;
        emit scanningChanged();
    });

    connect(scanner_, &BleScannerWorker::deviceFound,
            this, &BleManager::deviceFound);

    scannerThread_.start();
}

BleManager::~BleManager()
{
    stopScan();
    scannerThread_.quit();
    scannerThread_.wait();
}

void BleManager::startScan()
{
    if (scanning_) return;

    QMetaObject::invokeMethod(scanner_, "startScan",
                              Qt::QueuedConnection);
}

void BleManager::stopScan()
{
    if (!scanning_) return;

    QMetaObject::invokeMethod(scanner_, "stopScan",
                              Qt::QueuedConnection);
}


/* [] END OF FILE */
