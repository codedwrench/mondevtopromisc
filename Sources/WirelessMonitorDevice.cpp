#include "../Includes/WirelessMonitorDevice.h"


#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

#include "../Includes/Logger.h"
#include "../Includes/PCapReader.h"


using namespace std::chrono;

bool WirelessMonitorDevice::Open(const std::string& aName)
{
    bool lReturn{true};
    char lErrorBuffer[PCAP_ERRBUF_SIZE];

    mHandler = pcap_create(aName.c_str(), lErrorBuffer);
    pcap_set_rfmon(mHandler, 1);
    pcap_set_snaplen(mHandler, cSnapshotLength);
    pcap_set_promisc(mHandler, 1);
    pcap_set_timeout(mHandler, cTimeout);

    int lStatus;
    lStatus = pcap_activate(mHandler);

    if (lStatus != 0) {
        lReturn = false;
        Logger::GetInstance().Log("pcap_activate failed, " + std::string(pcap_statustostr(lStatus)), Logger::ERR);
    }
    return lReturn;
}

void WirelessMonitorDevice::Close()
{
    pcap_close(mHandler);
    mData = nullptr;
    mHeader = nullptr;
}

bool WirelessMonitorDevice::ReadNextPacket()
{
    bool lReturn{false};

    if (pcap_next_ex(mHandler, &mHeader, &mData) > 0) {
        std::string lData = DataToString();
        if (mPacketConverter.Is80211Data(lData) &&
            (mBSSIDToFilter.empty() || mPacketConverter.IsForBSSID(lData, mBSSIDToFilter))) {
            ++mPacketCount;
            Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::TRACE);

            // Show the size in bytes of the packet
            Logger::GetInstance().Log("Packet size: " + std::to_string(mHeader->len) + " bytes", Logger::TRACE);


            // Show a warning if the length captured is different
            if (mHeader->len != mHeader->caplen) {
                Logger::GetInstance().Log("Capture size different than packet size:" + std::to_string(mHeader->len) +
                                          " bytes", Logger::WARNING);
            }

            // Show Epoch Time
            Logger::GetInstance().Log("Epoch time: " + std::to_string(mHeader->ts.tv_sec) + ":" +
                                      std::to_string(mHeader->ts.tv_usec), Logger::TRACE);

            lReturn = true;
        }
    }

    return lReturn;
}

const unsigned char* WirelessMonitorDevice::GetData()
{
    return mData;
}

const pcap_pkthdr* WirelessMonitorDevice::GetHeader()
{
    return mHeader;
}

std::string WirelessMonitorDevice::DataToFormattedString()
{
    std::stringstream lFormattedString;
    // Loop through the packet and print it as hexidecimal representations of octets
    for (unsigned int lCount = 0; lCount < mHeader->caplen; lCount++) {
        // Start printing on the next line after every 16 octets
        if (lCount % 16 == 0) {
            lFormattedString << std::endl;
        }

        lFormattedString << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(mData[lCount])
                         << " ";
    }

    return lFormattedString.str();
}

std::string WirelessMonitorDevice::DataToString()
{
    // Convert from char* to string
    std::string lData{};
    lData.reserve(mHeader->caplen);
    for (unsigned int lCount = 0; lCount < mHeader->caplen; lCount++) {
        lData += mData[lCount];
    }

    return lData;
}

void WirelessMonitorDevice::SetBSSIDFilter(std::string_view aBSSID)
{
    // Sadly a BPF filter won't work here, so for now just do filtering in userspace :(
    mBSSIDToFilter = std::string(aBSSID);
}

bool WirelessMonitorDevice::Send(std::string_view aData)
{
    return true;

}
