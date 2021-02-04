#pragma once

/* Copyright (c) 2021 [Rick de Bondt] - WirelessPSPPluginDevice.h
 *
 * This file contains functions to capture data from a wireless device in monitor mode.
 *
 * */

#include <memory>

#include <boost/thread.hpp>

#include "Handler80211.h"
#include "IConnector.h"
#include "IPCapDevice.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WifiInterfaceWindows.h"
#else
#include "WifiInterfaceLinuxBSD.h"
#endif

namespace WirelessPSPPluginDevice_Constants
{
    static constexpr unsigned int cSnapshotLength{65535};
    static constexpr unsigned int cTimeout{1};
}  // namespace WirelessPSPPluginDevice_Constants

using namespace WirelessPSPPluginDevice_Constants;

/**
 * Class which allows a wireless device in monitor mode to capture data and send wireless frames.
 */
class WirelessPSPPluginDevice : public IPCapDevice
{
public:
    void BlackList(uint64_t aMAC) override;
    void ClearMACBlackList();
    [[nodiscard]] bool IsMACBlackListed(uint64_t aMAC) const;

    void                 Close() override;
    std::string          DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
    const unsigned char* GetData() override;
    const pcap_pkthdr*   GetHeader() override;

    /**
     * Gets locked onto BSSID, this is the BSSID found when searching for beacon frames with the filtered SSID.
     * @return Locked onto BSSID/
     */
    uint64_t GetLockedBSSID();

    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) override;
    bool Send(std::string_view aData, bool aModifyData);
    bool Send(std::string_view aData) override;
    void SetConnector(std::shared_ptr<IConnector> aDevice) override;
    bool StartReceiverThread() override;

private:
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);
    void ShowPacketStatistics(const pcap_pkthdr* aHeader) const;
    
    std::vector<uint64_t>           mBlackList{};
    bool                            mConnected{false};
    std::shared_ptr<IConnector>     mConnector{nullptr};
    const unsigned char*            mData{nullptr};
    pcap_t*                         mHandler{nullptr};
    uint64_t                        mAdapterMACAddress{};
    const pcap_pkthdr*              mHeader{nullptr};
    unsigned int                    mPacketCount{0};
    std::shared_ptr<boost::thread>  mReceiverThread{nullptr};
    bool                            mSendReceivedData{false};
    std::shared_ptr<IWifiInterface> mWifiInterface{nullptr};
};
