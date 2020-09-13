#ifndef TAPDEVICE_H
#define TAPDEVICE_H

/* Copyright (c) 2020 [Rick de Bondt] - TapDevice.h
 *
 * This file contains an interface for wrappers around a tap device.
 *
 * */

#include <string>

#include <linux/if.h>

#include "ILinuxDevice.h"


class TapDevice : public ILinuxDevice
{
public:
    int AllocateDevice() override;
    int CreateDevice() override;
    int GetFd() override;

protected:
    int Open(const std::string& aDeviceNameAndPath, int aMode) override;
    int Close() override;

private:
    int         IoCtl(int aFd, unsigned long aRequest, char* aArgp) override;
    int         AllocateOldDevice();
    int         mFd{0};
    std::string mDeviceName{""};
};


#endif  // TAPDEVICE_H
