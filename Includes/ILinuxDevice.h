#ifndef ILINUXDEVICE_H
#define ILINUXDEVICE_H

/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains an interface for wrappers around a linux device, for example a TUN/TAP interface or socket.
 *
 * */

#include <string>


class ILinuxDevice {
public:

    /**
     * Takes steps required for initializing a device.
     * @return 0 on success, errno on failure.
     */
    virtual int AllocateDevice() = 0;

    /**
     * Creates the device.
     * @return 0 on success, errno on failure.
     */
    virtual int CreateDevice() = 0;

    /**
     * Gets the filedescriptor of the device.
     * @return fd of the device, 0 on failure.
     */
    virtual int GetFd() = 0;

    /**
     * Closes the device
     * @return - 0 if successful, errno if unsuccessful.
     */
    virtual int Close() = 0;
protected:
    virtual int Open(const std::string& aDeviceNameAndPath, int aMode) = 0;
    virtual int IoCtl(int aFd, unsigned long aRequest, char* aArgp) = 0;
};


#endif //ILINUXDEVICE_H
