#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - WirelessMonitorDevice.h
 *
 * This file contains functions to capture data from a wireless device in monitor mode.
 *
 * */

#include <memory>

#include <boost/thread.hpp>

#include "IPCapDevice.h"
#include "PacketConverter.h"


namespace WirelessMonitorDevice_Constants
{
    static constexpr unsigned int cSnapshotLength{65535};
    static constexpr unsigned int cTimeout{10};
}  // namespace WirelessMonitorDevice_Constants

using namespace WirelessMonitorDevice_Constants;

/**
 * Class which allows a wireless device in monitor mode to capture data and send wireless frames.
 */
class WirelessMonitorDevice : public IPCapDevice
{
public:
    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter, uint16_t aFrequency) override;
    void Close() override;
    bool ReadNextData() override;
    const unsigned char* GetData() override;

    const pcap_pkthdr* GetHeader() override;

    std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
    std::string LastDataToString() override;

    void SetAcknowledgePackets(bool aAcknowledge);

    void SetSourceMACToFilter(uint64_t aMac);

    void SetSSID(std::string_view aSSID);

    // Only use if you're planning to use the internal wifi information
    bool Send(std::string_view aData) override;

    bool Send(std::string_view aData, IPCapDevice_Constants::WiFiBeaconInformation& aWiFiInformation) override;

    // Extra function which will make it possible to send without converting to 802.11 specifically.
    bool Send(std::string_view                              aData,
              IPCapDevice_Constants::WiFiBeaconInformation& aWiFiInformation,
              bool                                          aConvertData);

    void SetSendReceiveDevice(std::shared_ptr<ISendReceiveDevice> aDevice) override;

    // TODO: Put in ISendReceiveDevice, all types of devices can use this.
    /**
     * Starts receiving network messages from monitor device.
     * @return True if successful.
     */
    bool StartReceiverThread();

private:
    bool                                         ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);
    bool                                         mSendReceivedData{false};
    bool                                         mConnected{false};
    bool                                         mAcknowledgePackets{false};
    PacketConverter                              mPacketConverter{true};
    uint64_t                                     mSourceMACToFilter{0};
    const unsigned char*                         mData{nullptr};
    std::vector<std::string>                     mSSIDFilter;
    pcap_t*                                      mHandler{nullptr};
    const pcap_pkthdr*                           mHeader{nullptr};
    unsigned int                                 mPacketCount{0};
    std::shared_ptr<ISendReceiveDevice>          mSendReceiveDevice{nullptr};
    std::shared_ptr<boost::thread>               mReceiverThread{nullptr};
    IPCapDevice_Constants::WiFiBeaconInformation mWifiInformation{};
};
