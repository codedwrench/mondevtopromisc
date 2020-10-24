#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Logger.h
 *
 * This file converts packets from a monitor format to promiscuous format and vice versa.
 *
 **/

#include <array>
#include <string>

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/RadioTapReader.h"
#include "IPCapDevice.h"
#include "NetworkingHeaders.h"

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
     * Preload data about this packet into this class.
     * @param aData - Packet to dissect.
     */
    void Update(std::string_view aData);

    /**
     * Check if the provided data is part of a beacon packet.
     * Only works on packets containing a 802.11 header.
     * @param aData - The data to check.
     * @return true if packet is a beacon packet.
     */
    bool Is80211Beacon(std::string_view aData);

    /**
     * Tries to find an SSID in a beacon frame.
     * Note: Only works is this packet is a beacon frame.
     * @return string containing the SSID of this beacon frame. Empty string if not found.
     */
    std::string GetBeaconSSID(std::string_view aData);

    /**
     * Tries to find an BSSID in a beacon frame.
     * Note: Only works is this packet is a beacon frame.
     * @return uint64_t containing the BSSID of this beacon frame. 0 if not found.
     */
    uint64_t GetBSSID(std::string_view aData);

    /**
     * Tries to find the source mac in an 802.11 frame.
     * Note: Only works is this packet is a data frame.
     * @return uint64_t containing source mac of this data frame. 0 if not found.
     */
    uint64_t GetSourceMac(std::string_view aData);

    /**
     * Tries to find the destination mac in an 802.11 frame.
     * Note: Only works is this packet is a data frame.
     * @return uint64_t containing destination mac of this data frame. 0 if not found.
     */
    uint64_t GetDestinationMac(std::string_view aData);

    /**
     * Reads WiFi information from the 802.11 Wireless Management header in a beacon frame.
     * @param aWifiInfo - Wireless information to fill.
     * @return true if successful.
     */
    bool Fill80211WiFiInformation(std::string_view aData, IPCapDevice_Constants::WiFiBeaconInformation& aWifiInfo);

    /**
     * Checks if the provided data is part of a data packet.
     * Only works on packets containing a 802.11 header.
     * @param aData - The data to check.
     * @return true if packet is a data packet.
     */
    bool Is80211Data(std::string_view aData);

    /**
     * Checks if the provided data is part of a quality of service packet.
     * Only works on packets containing a 802.11 header.
     * @param aData - The data to check.
     * @return true if packet is a quality of service packet.
     */
    bool Is80211QOS(std::string_view aData);


    /**
     * Check if this packet is a retry packet so it can be skipped.
     * Only works on a 802.11 QOS packet.
     * @param aData - The data to check.
     * @return true if retry packet.
     */
    bool Is80211QOSRetry(std::string_view aData);


    /**
     * Checks if the provided data is part of a null function packet.
     * Only works on packets containing a 802.11 header.
     * @param aData - The data to check.
     * @return true if packet is a null function packet.
     */
    bool Is80211NullFunc(std::string_view aData);

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
     * @param aFrequency - The frequency to put in.
     * @param aMaxRate - The data rate to put in.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacketTo80211(std::string_view aData, uint64_t aBSSID, uint16_t aFrequency, uint8_t aMaxRate);

    /**
     * Sets whether this converter should convert keeping a radiotap header in mind.
     * @param aRadioTap - whether the converter should construct packets keeping a radiotap header in mind or add a
     * radiotap header
     */
    void SetRadioTap(bool aRadioTap);

    /**
     * Creaetes an acknowledgement frame based on MAC-address.
     * @param aReceiverMac - MAC address to fill in.
     * @param aFrequency - Frequency to fill in.
     * @param aMaxRate  - Max rate to fill in.
     * @return A string with the full packet.
     */
    std::string ConstructAcknowledgementFrame(std::array<uint8_t, 6> aReceiverMac,
                                              uint16_t               aFrequency,
                                              uint8_t                aMaxRate);

    /**
     * Checks if Source Mac is the same as the given MAC.
     * @param aData - The packet.
     * @param aMac - The Mac to check for.
     * @return true if match.
     */
    bool IsFromMac(std::string_view aData, uint64_t aMac);


private:
    void InsertRadioTapHeader(char* aPacket, uint16_t aFrequency, uint8_t aMaxRate) const;
    int  FillSSID(std::string_view aData, IPCapDevice_Constants::WiFiBeaconInformation& aWifiInfo, uint64_t aIndex);

    RadioTapReader mRadioTapReader{};
    bool           mRadioTap{false};
};
