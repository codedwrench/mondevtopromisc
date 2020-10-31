#pragma once

// TODO: This is almost a carbon copy of monitordevice, make base?
/* Copyright (c) 2020 [Rick de Bondt] - PCapReader.h
 *
 * This file contains the necessary components to read a PCap file.
 *
 **/

#include <memory>

#include "Handler80211.h"
#include "Handler8023.h"
#include "IConnector.h"
#include "IPCapDevice.h"
#include "XLinkKaiConnection.h"

/**
 * This class contains the necessary components to read a PCap file.
 **/
class PCapReader : public IPCapDevice
{
public:
    /**
     * Construct a new PCapReader object.
     * @param aMonitorCapture - Tells the PCapReader whether it's a monitor mode device or a promiscuous mode device.
     */
    explicit PCapReader(bool aMonitorCapture, bool aTimeAccurate);
    ~PCapReader() = default;

    void                 Close() override;
    std::string          DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
    const unsigned char* GetData() override;
    const pcap_pkthdr*   GetHeader() override;
    [[nodiscard]] bool   IsDoneReceiving() const;
    bool                 Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) override;
    // PCapReader should still be able to manually read next data, way too useful to private.
    bool ReadNextData();
    bool Send(std::string_view aData) override;
    void SetAcknowledgePackets(bool aAcknowledge);
    void SetConnector(std::shared_ptr<IConnector> aDevice) override;
    void SetSourceMACToFilter(uint64_t aMac);
    // In this case tries to simulate a real device
    bool StartReceiverThread() override;

private:
    bool ReadCallback(const unsigned char* aData, pcap_pkthdr* aHeader);
    void ShowPacketStatistics(pcap_pkthdr* aHeader) const;

    bool                           mAcknowledgePackets{false};
    bool                           mConnected{false};
    std::shared_ptr<IConnector>    mConnector{nullptr};
    const unsigned char*           mData{nullptr};
    pcap_t*                        mHandler{nullptr};
    pcap_pkthdr*                   mHeader{nullptr};
    bool                           mDoneReceiving{false};
    bool                           mMonitorCapture{false};
    bool                           mTimeAccurate{false};
    std::shared_ptr<IHandler>      mPacketHandler{nullptr};
    std::shared_ptr<boost::thread> mReplayThread{nullptr};
    std::vector<std::string>       mSSIDFilter{};
    unsigned int                   mPacketCount{0};
};