#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - MonitorDevice.h
 *
 * This file contains functions to capture data from a wireless device in monitor mode.
 *
 * */

#include <memory>

#include <boost/thread.hpp>

#include "Handler80211.h"
#include "IConnector.h"
#include "IPCapDevice.h"


namespace WirelessMonitorDevice_Constants
{
    static constexpr unsigned int cSnapshotLength{65535};
    static constexpr unsigned int cTimeout{10};
}  // namespace WirelessMonitorDevice_Constants

using namespace WirelessMonitorDevice_Constants;

/**
 * Class which allows a wireless device in monitor mode to capture data and send wireless frames.
 */
class MonitorDevice : public IPCapDevice
{
public:
    /**
     * Adds a MAC address to the blacklist.
     * @param aMAC - MAC address to blacklist.
     */
    void                 BlackList(uint64_t aMAC);
    void                 Close() override;
    std::string          DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
    const unsigned char* GetData() override;
    const pcap_pkthdr*   GetHeader() override;

    /**
     * Gets parameters for a data packet type, for example used for conversion to an 80211 packet.
     * @return a reference to PhysicalDeviceParameters object with the needed parameters.
     */
    const RadioTapReader::PhysicalDeviceParameters& GetDataPacketParameters();

    /**
     * Gets locked onto BSSID, this is the BSSID found when searching for beacon frames with the filtered SSID.
     * @return Locked onto BSSID/
     */
    uint64_t GetLockedBSSID();

    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) override;
    bool Send(std::string_view aData) override;
    void SetAcknowledgePackets(bool aAcknowledge);
    void SetConnector(std::shared_ptr<IConnector> aDevice) override;
    void SetSourceMACToFilter(uint64_t aMac);
    bool StartReceiverThread() override;

private:
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);
    void ShowPacketStatistics(const pcap_pkthdr* aHeader) const;

    bool                           mAcknowledgePackets{false};
    bool                           mConnected{false};
    std::shared_ptr<IConnector>    mConnector{nullptr};
    const unsigned char*           mData{nullptr};
    pcap_t*                        mHandler{nullptr};
    const pcap_pkthdr*             mHeader{nullptr};
    unsigned int                   mPacketCount{0};
    Handler80211                   mPacketHandler{PhysicalDeviceHeaderType::RadioTap};
    std::shared_ptr<boost::thread> mReceiverThread{nullptr};
    bool                           mSendReceivedData{false};
};
