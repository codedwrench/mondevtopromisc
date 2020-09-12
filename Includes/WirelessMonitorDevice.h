#ifndef WIRELESS_MONITOR_DEVICE_H
#define WIRELESS_MONITOR_DEVICE_H

/* Copyright (c) 2020 [Rick de Bondt] - WirelessMonitorDevice.h
 *
 * This file contains functions to capture data from a wireless device in monitor mode.
 *
 * */

#include "IPCapDevice.h"
#include "PacketConverter.h"


namespace WirelessMonitorDevice_Constants
{
    constexpr unsigned int cSnapshotLength{2048};
    constexpr unsigned int cTimeout{512};
}  // namespace WirelessMonitorDevice_Constants

using namespace WirelessMonitorDevice_Constants;

/**
 * Class which allows a wireless device in monitor mode to capture data and send wireless frames.
 */
class WirelessMonitorDevice : public IPCapDevice
{
public:
    // TODO: If too many copied functions, create base class.
    void                 Close() override;
    bool                 Open(const std::string& aName) override;
    bool                 ReadNextPacket() override;
    const unsigned char* GetData() override;

    const pcap_pkthdr* GetHeader() override;

    std::string DataToFormattedString() override;

    std::string DataToString() override;

    void SetBSSID(std::string_view aBSSID) override;

    bool Send(std::string_view aData) override;

private:
    PacketConverter      mPacketConverter{true};
    const unsigned char* mData{nullptr};
    std::string          mBSSID;
    pcap_t*              mHandler{nullptr};
    pcap_pkthdr*         mHeader{nullptr};
    unsigned int         mPacketCount{0};
};


#endif  // WIRELESS_MONITOR_DEVICE_H
