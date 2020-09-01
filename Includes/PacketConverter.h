#ifndef PACKETCONVERTER_H
#define PACKETCONVERTER_H

/* Copyright (c) 2020 [Rick de Bondt] - Logger.h
 *
 * This file converts packets from a monitor format to promiscuous format.
 *
 **/

#include <string_view>


namespace PacketConverter_Constants
{
    constexpr unsigned int cDestinationAddressIndex{4};
    constexpr unsigned int cDestinationAddressLength{6};
    constexpr unsigned int cSourceAddressIndex{10};
    constexpr unsigned int cSourceAddressLength{6};
    constexpr unsigned int cBSSIDIndex{16};
    constexpr unsigned int cBSSIDLength{6};
    constexpr unsigned int cTypeIndex{30};
    constexpr unsigned int cTypeLength{2};
    constexpr unsigned int cDataIndex{32};
    constexpr unsigned int cRadioTapLengthIndex{2};
    constexpr uint8_t c80211DataType{0x08};
    constexpr unsigned int cHeaderLength{cDestinationAddressLength +
                                         cSourceAddressLength +
                                         cTypeLength};
}

using namespace PacketConverter_Constants;

class PacketConverter
{
public:
    /**
     * Constructs a PacketConverter that converts packets from a wireless (radiotap + 802.11) format to an ethernet,
     * (802.3) format.
     * @param aHasRadioTap
     */
    PacketConverter(bool aHasRadioTap = false);

    /**
     * Checks if the provided data is part of a data packet.
     * Only works on packets containing a 802.11 header.
     */
    bool Is80211Data(std::string_view aData);

    /**
     * Check if provided packet matches BSSID.
     * @param aData - Data to inspect.
     * @param aBSSID - BSSID to compare against.
     * @return true if packet is for this BSSID.
     */
    bool IsForBSSID(std::string_view aData, std::string_view aBSSID);

    /**
     * This function converts a monitor mode packet to a promiscuous mode packet, stripping the radiotap and
     * 802.11 header and adding an 802.3 header. Only converts data packets!
     * @param aData - The packet data to convert.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacketToPromiscuous(std::string_view aData);
private:

    void UpdateIndexAfterRadioTap(std::string_view aData);
    bool mHasRadioTap{false};
    uint16_t mIndexAfterRadioTap{0};
};


#endif //PACKETCONVERTER_H
