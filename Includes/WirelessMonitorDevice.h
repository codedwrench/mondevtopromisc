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
    constexpr unsigned int cSnapshotLength{2048};
    constexpr unsigned int cTimeout{10};
}  // namespace WirelessMonitorDevice_Constants

using namespace WirelessMonitorDevice_Constants;

/**
 * Class which allows a wireless device in monitor mode to capture data and send wireless frames.
 */
class WirelessMonitorDevice : public IPCapDevice
{
public:
    void                 Close() override;
    bool                 Open(std::string_view aName) override;
    bool                 ReadNextData() override;
    const unsigned char* GetData() override;

    const pcap_pkthdr* GetHeader() override;

    std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
    std::string LastDataToString() override;

    void SetBSSID(std::string_view aBSSID) override;

    bool Send(std::string_view aData) override;

    void SetSendReceiveDevice(std::shared_ptr<ISendReceiveDevice> aDevice) override;

    // TODO: Put in ISendReceiveDevice, all types of devices can use this.
    /**
     * Starts receiving network messages from monitor device.
     * @return True if successful.
     */
    bool StartReceiverThread();

private:
    bool                                ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);
    bool                                mSendReceivedData{false};
    bool                                mConnected{false};
    PacketConverter                     mPacketConverter{true};
    const unsigned char*                mData{nullptr};
    uint64_t                            mBSSID;
    pcap_t*                             mHandler{nullptr};
    const pcap_pkthdr*                  mHeader{nullptr};
    unsigned int                        mPacketCount{0};
    std::shared_ptr<ISendReceiveDevice> mSendReceiveDevice{nullptr};
    std::shared_ptr<boost::thread>      mReceiverThread{nullptr};
};
