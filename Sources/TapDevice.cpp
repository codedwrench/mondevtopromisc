#include <fcntl.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "../Includes/TapDevice.h"

int TapDevice::Open(const std::string& device_name_and_path, int mode)
{
    int lReturn = open(device_name_and_path.c_str(), mode);
    if (lReturn != -1) {
        mFd = lReturn;
        lReturn = 0;
    } else {
        lReturn = errno;
    }

    return lReturn;
}

int TapDevice::Close()
{
    int lReturn = close(mFd);
    mFd = 0;
    return lReturn;
}

int TapDevice::IoCtl(int aFd, unsigned long aRequest, char* aArgp)
{
    return ioctl(aFd, aRequest, aArgp);
}

int TapDevice::CreateDevice(const std::string& aDeviceName)
{
    // Create device
    ifreq lIfr {0};

    // Clear lIfr with memset to be sure it's actually empty
    memset(&lIfr, 0, sizeof(lIfr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    lIfr.ifr_flags = IFF_TAP;
    if (!aDeviceName.empty())
    {
        strncpy(lIfr.ifr_name, aDeviceName.c_str(), IFNAMSIZ);
    }

    int lReturn = IoCtl(mFd, TUNSETIFF, reinterpret_cast<char *>(&lIfr));

    if (lReturn != -1) {
        mDeviceName = lIfr.ifr_name;
        lReturn = 0;
    } else {
        Close();
        lReturn = errno;
        std::cout << "Failed to create device: " << strerror(lReturn) << std::endl;
    }

    return lReturn;
}

int TapDevice::AllocateDevice()
{
    int lReturn = 0;

    int lError = Open("/dev/net/tun", O_RDWR);
    if (lError != 0) {
        std::cout << "Failed to open device" << strerror(lError) << std::endl;
    }

    if (mFd < 0) {
        return AllocateOldDevice();
    }

    return lReturn;
}

int TapDevice::AllocateOldDevice() {
    std::string device_name_and_path {"/dev/" + mDeviceName};
    return Open(device_name_and_path, O_RDWR);
}

int TapDevice::GetFd() {
    return mFd;
}