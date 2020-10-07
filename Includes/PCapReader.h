#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains the necessary components to read a PCap file.
 *
 * */

#include "IPCapDevice.h"
#include "PacketConverter.h"
#include "XLinkKaiConnection.h"

/**
 * This class contains the necessary components to read a PCap file.
 * */
class PCapReader : public IPCapDevice
{
public:
    void Close() override;

    bool Open(std::string_view aName);

    bool ReadNextData() override;

    const unsigned char* GetData() override;

    const pcap_pkthdr* GetHeader() override;

    std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) override;

    std::string LastDataToString() override;

    // TODO: Implement
    void SetBSSID(uint64_t aBSSID) override{};

    bool Send(std::string_view aData) override;

    /**
     * Replays packets from file to injection device / XLink Kai.
     * Assumes no packets have been read from the opened file yet.
     * Tip: Put into separate thread for better timing accuracy.
     * @param aMonitorCapture - If the file was captured in monitor mode.
     * @param aHasRadioTap - If the file has radiotap headers.
     * @return pair with amount of packets sent and whether it has fully replayed them or not.
     */
    std::pair<bool, unsigned int> ReplayPackets(bool aMonitorCapture = false, bool aHasRadioTap = false);

    void SetSendReceiveDevice(std::shared_ptr<ISendReceiveDevice> aDevice) override;

private:
    /**
     * Constructs and replays a packet to given interface.
     * @param aConnection - Connection to send the packet on.
     * @param aPacketConverter - A PacketConverter object.
     * @param aMonitorCapture - Whether the capture was made in monitor mode.
     * @return a pair containing, succesfully sent (or ignored) and whether the packet was useful enough to be sent.
     */
    std::pair<bool, bool> ConstructAndReplayPacket(ISendReceiveDevice& aConnection,
                                                   PacketConverter     aPacketConverter,
                                                   bool                aMonitorCapture);

    const unsigned char*                mData{nullptr};
    pcap_t*                             mHandler{nullptr};
    pcap_pkthdr*                        mHeader{nullptr};
    unsigned int                        mPacketCount{0};
    std::shared_ptr<ISendReceiveDevice> mSendReceiveDevice{nullptr};
};
