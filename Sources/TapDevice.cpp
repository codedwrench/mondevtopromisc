#include "../Includes/TapDevice.h"

#include <cstring>
#include <iostream>

#include <linux/if_tun.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <unistd.h>

#include "../Includes/Logger.h"

int TapDevice::Open(const std::string& aDeviceNameAndPath, int aMode)
{
    int lReturn = open(aDeviceNameAndPath.c_str(), aMode);
    if (lReturn != -1) {
        mFd     = lReturn;
        lReturn = 0;
    } else {
        lReturn = errno;
    }

    return lReturn;
}

int TapDevice::Close()
{
    int lReturn = close(mFd);
    mFd         = 0;
    return lReturn;
}

int TapDevice::IoCtl(int aFd, unsigned long aRequest, char* aArgp)
{
    return ioctl(aFd, aRequest, aArgp);
}

int TapDevice::CreateDevice()
{
    // Create device
    ifreq lIfr{0};

    // Clear lIfr with memset to be sure it's actually empty
    memset(&lIfr, 0, sizeof(lIfr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    lIfr.ifr_flags = IFF_TAP;

    int lReturn = IoCtl(mFd, TUNSETIFF, reinterpret_cast<char*>(&lIfr));

    if (lReturn != -1) {
        mDeviceName = lIfr.ifr_name;
        lReturn     = 0;
    } else {
        Close();
        lReturn = errno;
        Logger::GetInstance().Log("Failed to create device, " + std::string(strerror(lReturn)), Logger::ERR);
    }

    return lReturn;
}

int TapDevice::AllocateDevice()
{
    int lReturn = 0;

    lReturn = Open("/dev/net/tun", O_RDWR);
    if (lReturn != 0) {
        Logger::GetInstance().Log("Failed to open device, " + std::string(strerror(lReturn)), Logger::ERR);
    }

    if (mFd < 0) {
        return AllocateOldDevice();
    }

    return lReturn;
}

int TapDevice::AllocateOldDevice()
{
    std::string lDeviceNameAndPath{"/dev/" + mDeviceName};
    return Open(lDeviceNameAndPath, O_RDWR);
}

int TapDevice::GetFd()
{
    return mFd;
}
