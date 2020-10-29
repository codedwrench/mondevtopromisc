#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - NetConversionFunctions.h
 *
 * This file contains some general conversion functions for network related things.
 *
 **/

#if defined(_MSC_VER) or defined(__MINGW32__)
#include <stdlib.h>
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)
#else
#include <byteswap.h>  // bswap_16 bswap_32 bswap_64
#endif

#include <cstring>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "RadioTapReader.h"

/**
 * Converts a channel to frequency, only 2.4Ghz frequencies supported.
 * @param aChannel - Channel to convert,
 * @return Channel as frequency.
 */
static int ConvertChannelToFrequency(int aChannel)
{
    int lReturn{-1};

    // 2.4GHz, steps of 5hz.
    if (aChannel >= 1 && aChannel <= 13) {
        lReturn = 2412 + ((aChannel - 1) * 5);
    }

    return lReturn;
}

/**
 * Helper function to get raw data more easily.
 * @param aPacket - Packet to grab data from.
 */
template<typename Type> static Type GetRawData(std::string_view aPacket, unsigned int aIndex)
{
    return (*reinterpret_cast<const Type*>(aPacket.data() + aIndex));
}

/**
 * Helper function to get raw data as string more easily.
 */
static std::string GetRawString(std::string_view aPacket, unsigned int aIndex, unsigned int aLength)
{
    const char* lData{reinterpret_cast<const char*>(aPacket.data() + aIndex)};
    return std::string(lData, aLength);
}

/**
 * Swaps endianness of Mac.
 * @param aMac - Mac to swap.
 * @return swapped mac.
 */
static uint64_t SwapMacEndian(uint64_t aMac)
{
    // Little- to Big endian
    aMac = bswap_64(aMac);
    return aMac >> 16U;
}

/**
 * Helper function for ConvertPacket, adds the IEEE80211 Header.
 * This one is based on Ad-Hoc traffic.
 * @param aData - Data to insert the Header to
 * @param aSourceAddress - Source MAC to insert.
 * @param aDestinationAddress - Destination MAC to insert.
 * @param aBSSID - BSSID to insert.
 * @param aIndex - Index to insert the header at.
 */
static void InsertIEEE80211Header(
    char* aPacket, uint64_t aSourceAddress, uint64_t aDestinationAddress, uint64_t aBSSID, uint8_t aIndex)
{
    ieee80211_hdr lIeee80211Header{};
    memset(&lIeee80211Header, 0, sizeof(lIeee80211Header));

    lIeee80211Header.frame_control = Net_80211_Constants::cWlanFCTypeData;
    lIeee80211Header.duration_id   = 0xffff;  // Just an arbitrarily high number.

    // For Ad-Hoc
    //  | Address 1   | Address 2   | Address 3   | Address 4 |
    //  +-------------+-------------+-------------+-----------+
    //  | Destination | Source      | BSSID       | N/A       |

    memcpy(&lIeee80211Header.addr1[0],
           reinterpret_cast<const char*>(&aDestinationAddress),
           Net_80211_Constants::cDestinationAddressLength * sizeof(uint8_t));

    memcpy(&lIeee80211Header.addr2[0],
           reinterpret_cast<const char*>(&aSourceAddress),
           Net_80211_Constants::cSourceAddressLength * sizeof(uint8_t));

    uint64_t lBSSID = aBSSID;

    // Little- to Big endian
    lBSSID = SwapMacEndian(lBSSID);
    memcpy(&lIeee80211Header.addr3[0], &lBSSID, Net_80211_Constants::cBSSIDLength * sizeof(uint8_t));

    memcpy(aPacket + aIndex, &lIeee80211Header, sizeof(lIeee80211Header));
}


/**
 * Helper function that inserts a radiotap header into a packet.
 * @param aPacket - Packet to insert radiotap header into.
 * @param aParameters - Parameters to use when inserting the parameters.
 * @return size of radiotap header.
 */
static int InsertRadioTapHeader(char* aPacket, RadioTapReader::PhysicalDeviceParameters aParameters)
{
    unsigned int lIndex{sizeof(RadioTapHeader)};

    // RadioTap Header
    RadioTapHeader lRadioTapHeader{};
    memset(&lRadioTapHeader, 0, sizeof(lRadioTapHeader));

    // General header
    lRadioTapHeader.present_flags   = RadioTap_Constants::cSendPresentFlags;
    lRadioTapHeader.bytes_in_header = RadioTap_Constants::cRadioTapSize;

    if (aParameters.mKnownMCSInfo != 0) {
        // Clear bit 2, DataRate
        lRadioTapHeader.present_flags = lRadioTapHeader.present_flags & ~uint32_t(1U << 2U);

        // Set bit 19, MCS info
        lRadioTapHeader.present_flags = lRadioTapHeader.present_flags & uint32_t(1U << 19U);

        // Add 2 to the radiotap size (-1 for the data rate + 3 for the MCS info)
        lRadioTapHeader.bytes_in_header += 2;
    }
    memcpy(aPacket, &lRadioTapHeader, sizeof(lRadioTapHeader));

    // Optional header (Flags)
    uint8_t lFlags{aParameters.mFlags};

    // If FCS at end set, reset
    lFlags = (lFlags & ~uint8_t(0x10U));

    memcpy(aPacket + lIndex, &lFlags, sizeof(lFlags));
    lIndex += sizeof(lFlags);

    if (aParameters.mKnownMCSInfo == 0) {
        // Optional header (Rate Flags)
        uint8_t lRateFlags{aParameters.mDataRate};
        memcpy(aPacket + lIndex, &lRateFlags, sizeof(lRateFlags));
    }

    // Even if no datarate, there needs to be padding
    lIndex += sizeof(uint8_t);

    // Optional headers (Channel & Channel Flags)
    uint16_t lChannel{aParameters.mFrequency};
    memcpy(aPacket + lIndex, &lChannel, sizeof(lChannel));
    lIndex += sizeof(lChannel);

    uint16_t lChannelFlags{aParameters.mChannelFlags};
    memcpy(aPacket + lIndex, &lChannelFlags, sizeof(lChannelFlags));
    lIndex += sizeof(lChannelFlags);

    // Optional header (TX Flags)
    uint32_t lTXFlags{RadioTap_Constants::cTXFlags};
    memcpy(aPacket + lIndex, &lTXFlags, sizeof(lTXFlags));
    lIndex += sizeof(lTXFlags);

    // Optional header (MCS), only add if found in parameter set (older wifi devices like the PSP don't use it)
    if (aParameters.mKnownMCSInfo != 0) {
        memcpy(aPacket + lIndex, &aParameters.mKnownMCSInfo, sizeof(aParameters.mKnownMCSInfo));
        lIndex += sizeof(aParameters.mKnownMCSInfo);
        memcpy(aPacket + lIndex, &aParameters.mMCSFlags, sizeof(aParameters.mMCSFlags));
        lIndex += sizeof(aParameters.mMCSFlags);
        memcpy(aPacket + lIndex, &aParameters.mMCSInfo, sizeof(aParameters.mMCSInfo));
    }

    return lRadioTapHeader.bytes_in_header;
}

/**
 * Converts a mac address int to string format: (xx:xx:xx:xx:xx:xx)
 * @param aMac - The mac address string to convert to an int.
 * @return std::string with the converted mac address.
 */
static std::string IntToMac(uint64_t aMac)
{
    std::ostringstream lOutput{};

    for (unsigned int lCount = 0; lCount < Net_80211_Constants::cSourceAddressLength; lCount++) {
        uint8_t lNibble{static_cast<uint8_t>(aMac >> (lCount * 8))};
        lOutput << std::hex << std::setfill('0') << std::setw(2) << (0xFF & lNibble);
        if (lCount != Net_80211_Constants::cSourceAddressLength - 1) {
            lOutput << ":";
        }
    }

    return lOutput.str();
}

/**
 * Converts a mac address string in format (xx:xx:xx:xx:xx:xx) to an int, has no safety build in for invalid
 * strings!
 * @param aMac - The mac address string to convert to an int.
 * @return int with the converted mac address.
 */
static uint64_t MacToInt(std::string_view aMac)
{
    uint64_t lResult{0};

    if (!aMac.empty()) {
        std::istringstream lStringStream(aMac.data());
        uint64_t           lNibble{0};
        lStringStream >> std::hex;
        while (lStringStream >> lNibble) {
            lResult = (lResult << 8U) + lNibble;
            lStringStream.get();
        }
    }

    return lResult;
}

/**
 * Creates an acknowledgement frame based on MAC-address.
 * @param aReceiverMac - MAC address to fill in.
 * @param aParameters - Parameters to use.
 * @return A string with the full packet.
 */
static std::string ConstructAcknowledgementFrame(uint64_t                                 aReceiverMac,
                                                 RadioTapReader::PhysicalDeviceParameters aParameters)
{
    std::string lReturn;

    // Big- to Little endian
    uint64_t lUnconvertedSourceMac = SwapMacEndian(aReceiverMac);

    std::array<uint8_t, 6> lSourceMac{};
    memcpy(reinterpret_cast<char*>(lSourceMac.data()), &lUnconvertedSourceMac, sizeof(uint8_t) * 6);

    unsigned int lReserveSize{sizeof(AcknowledgementHeader)};
    lReserveSize += RadioTap_Constants::cRadioTapSize;

    std::vector<char> lFullPacket;
    lFullPacket.reserve(lReserveSize);
    lFullPacket.resize(lReserveSize);

    unsigned int lIndex{0};

    // RadioTap Header
    InsertRadioTapHeader(&lFullPacket[0], aParameters);
    lIndex += RadioTap_Constants::cRadioTapSize;

    // Acknowledgement frame
    AcknowledgementHeader lAcknowledgementHeader{};
    memset(&lAcknowledgementHeader, 0, sizeof(lAcknowledgementHeader));

    lAcknowledgementHeader.frame_control = Net_80211_Constants::cAcknowledgementType;
    lAcknowledgementHeader.duration_id   = 0xffff;  // Just an arbitrarily high number.

    memcpy(&lAcknowledgementHeader.recv_address[0],
           lSourceMac.data(),
           Net_80211_Constants::cDestinationAddressLength * sizeof(uint8_t));

    memcpy(&lFullPacket[0] + lIndex, &lAcknowledgementHeader, sizeof(lAcknowledgementHeader));

    lReturn = std::string(lFullPacket.begin(), lFullPacket.end());
    return lReturn;
}

/**
 * Converts string to a pretty hex string for easy reading.
 * @param aData - Data to prettify.
 * @return prettified data as string.
 */
static std::string PrettyHexString(std::string_view aData)
{
    std::stringstream lFormattedString;

    // Start on new line immediately
    lFormattedString << std::endl;
    lFormattedString << "000000 ";

    // Loop through the packet and print it as hexidecimal representations of octets
    for (unsigned int lCount = 0; lCount < aData.size(); lCount++) {
        // Start printing on the next line after every 64 octets
        if ((lCount != 0) && (lCount % 64 == 0)) {
            lFormattedString << std::endl;
            lFormattedString << std::hex << std::setfill('0') << std::setw(6) << lCount << " ";
        } else if (lCount != 0) {
            lFormattedString << " ";
        }

        lFormattedString << std::hex << std::setfill('0') << std::setw(2) << (0xFF & aData.at(lCount));
    }

    return lFormattedString.str();
}