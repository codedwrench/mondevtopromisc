#pragma once

// TODO: This is almost a carbon copy of monitordevice, make base?
/* Copyright (c) 2020 [Rick de Bondt] - PCapReader.h
 *
 * This file contains the necessary components to read a PCap file.
 *
 **/

#include <memory>
#include <thread>

#include "Handler80211.h"
#include "Handler8023.h"
#include "IConnector.h"
#include "IPCapDevice.h"

/**
 * This class contains the necessary components to read a PCap file.
 **/
class PCapReader : public IPCapDevice, public IConnector
{
public:
    /**
     * Construct a new PCapReader object.
     * @param aMonitorCapture - Tells the PCapReader whether it's a monitor mode device or a promiscuous mode device.
     */
    explicit PCapReader(bool aMonitorCapture, bool aTimeAccurate);
    ~PCapReader() = default;

    /**
     * Adds a MAC address to the blacklist.
     * @param aMAC - MAC address to blacklist.
     */
    void BlackList(uint64_t aMAC);

    void        Close() override;
    std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;

    /**
     * Obtains BSSID, from monitor capture or previously given.
     * @return uint64_t with the BSSID obtained.
     */
    [[nodiscard]] uint64_t GetBSSID() const;

    const unsigned char* GetData() override;

    const pcap_pkthdr* GetHeader() override;

    /**
     * Obtains 80211 data packet parameters, from monitor capture or previously given .
     * @return data parameters that have been gotten from handler80211 or previously given.
     */
    std::shared_ptr<RadioTapReader::PhysicalDeviceParameters> GetDataParameters();

    /**
     * Get the packet handler class, this is kind of sneaky but its okay because this class is purely for testing
     * purposes anyway.
     * @return The handler.
     */
    std::shared_ptr<IHandler> GetPacketHandler();

    [[nodiscard]] bool IsDoneReceiving() const;
    bool               Open(std::string_view aArgument) override;
    bool               Open(std::string_view aName, std::vector<std::string>& aSSIDFilter) override;
    // PCapReader should still be able to manually read next data, way too useful to private.
    bool ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader);
    bool ReadNextData() override;
    bool Send(std::string_view aData) override;
    void SetAcknowledgePackets(bool aAcknowledge);
    void SetConnector(std::shared_ptr<IConnector> aDevice) override;
    void SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice) override;

    /**
     * Sets BSSID to use when pretending to be XLink Kai sending out to a monitor device.
     * @param aBSSID - BSSID to use.
     */
    void SetBSSID(uint64_t aBSSID);

    /**
     * Sets the Parameters to use when pretending to be XLink Kai sending out to a monitor device.
     * @param aParameters - Parameters to use.
     */
    void SetParameters(std::shared_ptr<RadioTapReader::PhysicalDeviceParameters> aParameters);

    void SetSourceMACToFilter(uint64_t aMac);
    // In this case tries to simulate a real device
    bool StartReceiverThread() override;

private:
    void ShowPacketStatistics(const pcap_pkthdr* aHeader) const;

    bool                                                      mAcknowledgePackets{false};
    uint64_t                                                  mBSSID{0};
    bool                                                      mConnected{false};
    std::shared_ptr<IConnector>                               mConnector{nullptr};
    const unsigned char*                                      mData{nullptr};
    std::shared_ptr<RadioTapReader::PhysicalDeviceParameters> mParameters{nullptr};
    pcap_t*                                                   mHandler{nullptr};
    pcap_pkthdr*                                              mHeader{nullptr};
    std::shared_ptr<IPCapDevice>                              mIncomingConnection{nullptr};
    bool                                                      mDoneReceiving{false};
    bool                                                      mMonitorCapture{false};
    bool                                                      mTimeAccurate{false};
    std::shared_ptr<IHandler>                                 mPacketHandler{nullptr};
    std::shared_ptr<std::thread>                              mReplayThread{nullptr};
    std::vector<std::string>                                  mSSIDFilter{};
    unsigned int                                              mPacketCount{0};
};