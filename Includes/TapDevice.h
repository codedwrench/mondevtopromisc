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
    int AllocateDevice() override;
    int CreateDevice(const std::string& aDeviceName) override;
    int GetFd() override ;

protected:
    int Open(const std::string& aDeviceNameAndPath, int aMode) override;
    int Close() override;

private:
    int IoCtl(int aFd, unsigned long aRequest, char* aArgp) override;
    int AllocateOldDevice();

    int mFd;
    std::string mDeviceName;

};


#endif //MONDEV_TAPDEVICE_H
