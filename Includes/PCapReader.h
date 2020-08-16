/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains the nessecary components to read a PCap file.
 *
 * */

#ifndef PCAPREADER_H
#define PCAPREADER_H

#include "IPCapDevice.h"

class PCapReader : public IPCapDevice {
public:
    void Close() override;
    bool Open(const std::string &aName) override;
    bool ReadNextPacket() override;
    const unsigned char *GetData() override;
    const pcap_pkthdr *GetHeader() override;

private:
    const unsigned char* mData = nullptr;
    pcap_t* mHandler = nullptr;
    pcap_pkthdr* mHeader = nullptr;
    unsigned int mPacketCount = 0;
};


#endif // PCAPREADER_H
