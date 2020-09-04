#ifndef PCAPREADER_H
#define PCAPREADER_H

/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains the necessary components to read a PCap file.
 *
 * */

#include "IPCapDevice.h"
#include "PacketConverter.h"
#include "XLinkKaiConnection.h"


class PCapReader : public IPCapDevice
{
public:
    void Close() override;

    bool Open(const std::string& aName) override;

    bool ReadNextPacket() override;

    const unsigned char* GetData() override;

    const pcap_pkthdr* GetHeader() override;

    std::string DataToFormattedString() override;

    //TODO: Implement
    void SetBSSIDFilter(std::string_view aBSSID) override
    {};

    bool Send(std::string_view aData) override;

    /**
     * Replays packets from file to injection device / XLink Kai.
     * Assumes no packets have been read from the opened file yet.
     * Tip: Put into separate thread for better timing accuracy.
     * @param aSendFunction - What to use to send the packet.
     * @param aMonitorCapture - If the file was captured in monitor mode.
     * @param aHasRadioTap - If the file has radiotap headers.
     * @return pair with amount of packets sent and whether it has fully replayed them or not.
     */
    std::pair<bool, unsigned int> ReplayPackets(XLinkKaiConnection& aConnection,
                                                bool aMonitorCapture = false,
                                                bool aHasRadioTap = false);

private:
    /**
     * Constructs and replays a packet to given interface.
     * @param aConnection - Connection to send the packet on.
     * @param aPacketConverter - A PacketConverter object.
     * @param aMonitorCapture - Whether the capture was made in monitor mode.
     * @return a pair containing, succesfully sent (or ignored) and whether the packet was useful enough to be sent.
     */
    std::pair<bool, bool> ConstructAndReplayPacket(XLinkKaiConnection& aConnection,
                                                   PacketConverter aPacketConverter,
                                                   bool aMonitorCapture);
    std::string DataToString();
    const unsigned char* mData{nullptr};
    pcap_t* mHandler{nullptr};
    pcap_pkthdr* mHeader{nullptr};
    unsigned int mPacketCount{0};
};


#endif // PCAPREADER_H
