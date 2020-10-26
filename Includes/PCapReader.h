#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains the necessary components to read a PCap file.
 *
 * */

// TODO: Rewrite
//#include "IPCapDevice.h"
//#include "XLinkKaiConnection.h"
//
///**
// * This class contains the necessary components to read a PCap file.
// * */
//class PCapReader : public IPCapDevice
//{
//public:
//    void Close() override;
//
//    // For promiscuous mode
//    bool Open(std::string_view aName, uint16_t aFrequency);
//
//    bool Open(std::string_view aName, std::vector<std::string>& aSSIDFilter, uint16_t aFrequency);
//
//    bool ReadNextData();
//
//    const unsigned char* GetData() override;
//
//    const pcap_pkthdr* GetHeader() override;
//
//    const IPCapDevice_Constants::WiFiBeaconInformation& GetWifiInformation();
//
//    std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;
//
//    std::string LastDataToString();
//
//    bool Send(std::string_view aData);
//
//    bool Send(std::string_view aData, IPCapDevice_Constants::WiFiBeaconInformation& aWiFiInformation);
//
//    /**
//     * Replays packets from file to injection device / XLink Kai.
//     * Assumes no packets have been read from the opened file yet.
//     * Tip: Put into separate thread for better timing accuracy.
//     * @param aMonitorCapture - If the file was captured in monitor mode.
//     * @param aHasRadioTap - If the file has radiotap headers.
//     * @return pair with amount of packets sent and whether it has fully replayed them or not.
//     */
//    std::pair<bool, unsigned int> ReplayPackets(bool aMonitorCapture = false, bool aHasRadioTap = false);
//
//    void SetConnector(std::shared_ptr<IConnector> aDevice) override;
//
//    /**
//     * Constructs and replays a packet to given interface.
//     * @param aData - The data to replay.
//     * @param aHeader - The header to replay.
//     * @param aPacketConverter - A PacketConverter object.
//     * @param aMonitorCapture - Whether the capture was made in monitor mode.
//     * @return a pair containing, succesfully sent (or ignored) and whether the packet was useful enough to be sent.
//     */
//    std::pair<bool, bool> ConstructAndReplayPacket(const unsigned char* aData,
//                                                   const pcap_pkthdr*   aHeader,
//                                                   PacketConverter      aPacketConverter,
//                                                   bool                 aMonitorCapture);
//
//private:
//    const unsigned char*                         mData{nullptr};
//    pcap_t*                                      mHandler{nullptr};
//    pcap_pkthdr*                                 mHeader{nullptr};
//    std::vector<std::string>                     mSSIDFilter{};
//    unsigned int                                 mPacketCount{0};
//    std::shared_ptr<IConnector>                  mSendReceiveDevice{nullptr};
//    IPCapDevice_Constants::WiFiBeaconInformation mWifiInformation{};
//};
