#ifndef TAPDEVICE_H
#define TAPDEVICE_H

/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains an interface for wrappers around a linux device, for example a TUN/TAP interface.
 *
 * */

#include <linux/if.h>
#include <string>

#include "ILinuxDevice.h"

class TapDevice : public ILinuxDevice {
public:
    virtual int AllocateDevice();
    virtual int CreateDevice(const std::string& aDeviceName);

    int GetFd();
protected:
    virtual int Open(const std::string& aDeviceNameAndPath, int aMode);

    virtual int Close();
private:
    virtual int IoCtl(int aFd, unsigned long aRequest, char* aArgp);
    int AllocateOldDevice();

    int mFd;
    std::string mDeviceName;

};


#endif //MONDEV_TAPDEVICE_H
