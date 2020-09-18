#ifndef PACKETCONVERTER_H
#define PACKETCONVERTER_H

/* Copyright (c) 2020 [Rick de Bondt] - Logger.h
 *
 * This file converts packets from a monitor format to promiscuous format and vice versa.
 *
 **/

#include <string_view>

/**
 * This class converts packets from a monitor format to promiscuous format and vice versa.
 */
class PacketConverter
{
public:
    /**
     * Constructs a PacketConverter that converts packets from a wireless (radiotap + 802.11) format to an ethernet,
     * (802.3) format.
     * @param aRadioTap - Whether packets sent to this function have a radiotap header or should have a radiotap header
     * added to them.
     */
    explicit PacketConverter(bool aRadioTap = false);

    /**
     * Converts a mac address string in format (xx:xx:xx:xx:xx:xx) to an int, has no safety build in for invalid
     * strings!
     * @param aMac - The mac address string to convert to an int.
     * @return int with the converted mac address.
     */
    static uint64_t MacToInt(std::string_view aMac);

    /**
     * Checks if the provided data is part of a data packet.
     * Only works on packets containing a 802.11 header.
     * @return true if packet is a data packet.
     */
    bool Is80211Data(std::string_view aData);

    /**
     * Checks if the provided data is part of a quality of service packet.
     * Only works on packets containing a 802.11 header.
     * @return true if packet is a quality of service packet.
     */
    bool Is80211QOS(std::string_view aData);

    /**
     * Check if provided packet matches BSSID.
     * @param aData - Data to inspect.
     * @param aBSSID - BSSID to compare against.
     * @return true if packet is for this BSSID.
     */
    bool IsForBSSID(std::string_view aData, uint64_t aBSSID);

    /**
     * This function converts a monitor mode packet to a promiscuous mode packet, stripping the radiotap and
     * 802.11 header and adding an 802.3 header. Only converts data packets!
     * @param aData - The packet data to convert.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacketTo8023(std::string_view aData);

    /**
     * This function converts a promiscuous mode packet to a monitor mode packet, adding the radiotap and
     * 802.11 header and removing the 802.3 header.
     * @param aData - The packet data to convert.
     * @param aBSSID - The BSSID to use when constructing the packet.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacketTo80211(std::string_view aData, uint64_t aBSSID);

    /**
     * Sets whether this converter should convert keeping a radiotap header in mind.
     * @param aRadioTap - whether the converter should construct packets keeping a radiotap header in mind or add a
     * radiotap header
     */
    void SetRadioTap(bool aRadioTap);

private:
    bool     UpdateIndexAfterRadioTap(std::string_view aData);
    bool     mRadioTap{false};
    uint64_t  mIndexAfterRadioTap{0};
};


#endif  // PACKETCONVERTER_H
