#ifndef BLEDEVICE_H
#define BLEDEVICE_H

#include <QString>

struct BleDevice {
    QString address;
    QString name;
    int rssi;
};

#endif // BLEDEVICE_H
