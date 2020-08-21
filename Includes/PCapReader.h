#ifndef PCAPREADER_H
#define PCAPREADER_H

/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains the necessary components to read a PCap file.
 *
 * */

#include "IPCapDevice.h"
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

    /**
     * Replays packets from file to injection device / XLink Kai.
     * Assumes no packets have been read from the opened file yet.
     * Tip: Put into separate thread for better timing accuracy.
     * @param aSendFunction - What to use to send the packet.
     * @return pair with amount of packets sent and whether it has fully replayed them or not.
     */
    std::pair<bool, unsigned int> ReplayPackets(XLinkKaiConnection& aConnection);

private:
    bool ReplayPacket(XLinkKaiConnection& aConnection);
    const unsigned char* mData{nullptr};
    pcap_t* mHandler{nullptr};
    pcap_pkthdr* mHeader{nullptr};
    unsigned int mPacketCount{0};
};


#endif // PCAPREADER_H
