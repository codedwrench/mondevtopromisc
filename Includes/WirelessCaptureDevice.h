#ifndef CAPTUREDEVICE_H
#define CAPTUREDEVICE_H

/* Copyright (c) 2020 [Rick de Bondt] - WirelessCaptureDevice.h
 *
 * This file contains functions to capture data from a wireless device in monitor mode.
 *
 * */

#include "IPCapDevice.h"
#include "PacketConverter.h"

class WirelessCaptureDevice : public IPCapDevice
{
public:
    // TODO: If too many copied functions, create base class.
    void Close();
    bool Open(const std::string& aName);
    bool ReadNextPacket();
    const unsigned char* GetData();
    const pcap_pkthdr* GetHeader();
    std::string DataToFormattedString();
    std::string DataToString();
    void SetBSSIDFilter(std::string_view aBSSID);
private:
    PacketConverter mPacketConverter{true};
    const unsigned char* mData{nullptr};
    std::string mBSSIDToFilter;
    pcap_t* mHandler{nullptr};
    pcap_pkthdr* mHeader{nullptr};
    unsigned int mPacketCount{0};
};


#endif //CAPTUREDEVICE_H
