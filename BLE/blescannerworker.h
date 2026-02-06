#ifndef BLESCANNERWORKER_H
#define BLESCANNERWORKER_H

#include <QObject>
#include <atomic>

#if defined(USE_SIMPLEBLE)
#include <simpleble/SimpleBLE.h>
#endif

class BleScannerWorker : public QObject
{
    Q_OBJECT
public:
    explicit BleScannerWorker(QObject *parent = nullptr);
    ~BleScannerWorker();
    void initAdapter();

public slots:
    void startScan();
    void stopScan();

signals:
    void deviceFound(QString address, QString name);
    void scanStarted();
    void scanStopped();

private:
#if defined(USE_SIMPLEBLE)
    std::unique_ptr<SimpleBLE::Adapter> adapter_;
#else
    void* adapter_;
#endif
    bool adapterInitialized_ = false; // Fixes: 'Use of undeclared identifier'
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> scanning_{false};
};

#endif // BLESCANNERWORKER_H

/* [] END OF FILE */
