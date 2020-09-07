#include "../Includes/PacketConverter.h"

#ifdef _MSC_VER
#include <stdlib.h>
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)
#else

#include <byteswap.h>  // bswap_16 bswap_32 bswap_64

#endif

#include <iostream>
#include <numeric>
#include <regex>
#include <string>

#include "../Includes/Logger.h"
#include "../Includes/NetworkingHeaders.h"


// Skip use of ether_aton because that could hinder Windows support.
uint64_t MacToInt(std::string_view aMac)
{
    std::istringstream lStringStream(aMac.data());
    uint64_t           lNibble{0};
    uint64_t           lResult{0};
    lStringStream >> std::hex;
    while (lStringStream >> lNibble) {
        lResult = (lResult << 8) + lNibble;
        lStringStream.get();
    }

    return lResult;
}

PacketConverter::PacketConverter(bool aHasRadioTap)
{
    mHasRadioTap = aHasRadioTap;
}

void PacketConverter::UpdateIndexAfterRadioTap(std::string_view aData)
{
    if (mHasRadioTap) {
        mIndexAfterRadioTap =
            *reinterpret_cast<const uint16_t*>(aData.data() + Net_80211_Constants::cRadioTapLengthIndex);
    }
}

bool PacketConverter::Is80211Data(std::string_view aData)
{
    bool lReturn{false};
    UpdateIndexAfterRadioTap(aData);

    // Sometimes it seems to send malformed packets. 255 is an arbitrary number.
    if (mIndexAfterRadioTap <= 255) {
        lReturn =
            *reinterpret_cast<const uint8_t*>(aData.data() + mIndexAfterRadioTap) == Net_80211_Constants::cDataType;
    } else {
        lReturn = false;
    }
    return lReturn;
}

bool PacketConverter::IsForBSSID(std::string_view aData, std::string_view aBSSID)
{
    UpdateIndexAfterRadioTap(aData);

    uint64_t lMac =
        *reinterpret_cast<const uint64_t*>(aData.data() + mIndexAfterRadioTap + Net_80211_Constants::cBSSIDIndex);
    lMac &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48u) - 1);  // it's actually a uint48.

    // Big- to Little endian
    lMac = bswap_64(lMac);
    lMac = lMac >> 16u;

    return lMac == MacToInt(aBSSID);
}

std::string PacketConverter::ConvertPacketTo8023(std::string_view aData)
{
    UpdateIndexAfterRadioTap(aData);

    std::string lConvertedPacket{};

    // The header should have it's complete size for the packet to be valid.
    if (aData.size() > Net_80211_Constants::cHeaderLength + mIndexAfterRadioTap) {
        // Strip framecheck sequence as well.
        lConvertedPacket.reserve(aData.size() - Net_80211_Constants::cDataIndex - mIndexAfterRadioTap -
                                 Net_80211_Constants::cFCSLength);

        lConvertedPacket.append(aData.substr(Net_80211_Constants::cDestinationAddressIndex + mIndexAfterRadioTap,
                                             Net_80211_Constants::cDestinationAddressLength));

        lConvertedPacket.append(aData.substr(Net_80211_Constants::cSourceAddressIndex + mIndexAfterRadioTap,
                                             Net_80211_Constants::cSourceAddressLength));

        lConvertedPacket.append(
            aData.substr(Net_80211_Constants::cTypeIndex + mIndexAfterRadioTap, Net_80211_Constants::cTypeLength));

        lConvertedPacket.append(aData.substr(
            Net_80211_Constants::cDataIndex + mIndexAfterRadioTap,
            aData.size() - mIndexAfterRadioTap - Net_80211_Constants::cDataIndex - Net_80211_Constants::cFCSLength));
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet", Logger::WARNING);
    }

    // [ Destination MAC | Source MAC | EtherType ] [ Payload ]
    return lConvertedPacket;
}

// Helper function for ConvertPacketTo80211, adds the radiotap header.
void InsertRadioTapHeader(std::string_view aData, char* aPacket)
{
    unsigned int lIndex{sizeof(RadioTapHeader)};

    // RadioTap Header
    RadioTapHeader lRadioTapHeader{};
    memset(&lRadioTapHeader, 0, sizeof(lRadioTapHeader));

    // General header
    lRadioTapHeader.present_flags   = RadioTap_Constants::cSendPresentFlags;
    lRadioTapHeader.bytes_in_header = sizeof(lRadioTapHeader) + sizeof(RadioTap_Constants::cFlags) +
                                      sizeof(RadioTap_Constants::cChannel) + sizeof(RadioTap_Constants::cChannelFlags) +
                                      sizeof(RadioTap_Constants::cRateFlags) + sizeof(RadioTap_Constants::cTXFlags);

    memcpy(aPacket, &lRadioTapHeader, sizeof(lRadioTapHeader));

    // Optional header (Flags)
    uint8_t lFlags{RadioTap_Constants::cFlags};
    memcpy(aPacket + lIndex, &lFlags, sizeof(lFlags));
    lIndex += sizeof(lFlags);

    // Optional header (Rate Flags)
    uint8_t lRateFlags{RadioTap_Constants::cRateFlags};
    memcpy(aPacket + lIndex, &lRateFlags, sizeof(lRateFlags));
    lIndex += sizeof(lRateFlags);

    // Optional headers (Channel & Channel Flags)
    uint16_t lChannel{RadioTap_Constants::cChannel};
    memcpy(aPacket + lIndex, &lChannel, sizeof(lChannel));
    lIndex += sizeof(lChannel);

    uint16_t lChannelFlags{RadioTap_Constants::cChannelFlags};
    memcpy(aPacket + lIndex, &lChannelFlags, sizeof(lChannelFlags));
    lIndex += sizeof(lChannelFlags);

    // Optional header (TX Flags)
    uint32_t lTXFlags{RadioTap_Constants::cTXFlags};
    memcpy(aPacket + lIndex, &lTXFlags, sizeof(lTXFlags));
}

// Helper function for ConvertPacketTo80211, adds the ieee80211 header.
void InsertIeee80211Header(std::string_view aData, std::string_view aBSSID, char* aPacket, unsigned int aPacketIndex)
{
    // Then comes the IEEE80211 header
    ieee80211_hdr lIeee80211Header{};
    memset(&lIeee80211Header, 0, sizeof(lIeee80211Header));

    lIeee80211Header.frame_control = Net_80211_Constants::cWlanFCTypeData;
    lIeee80211Header.duration_id   = 0xffff;  // Just an arbitrarily high number.

    // For Ad-Hoc
    //  | Address 1   | Address 2   | Address 3   | Address 4 |
    //  +-------------+-------------+-------------+-----------+
    //  | Destination | Source      | BSSID       | N/A       |

    memcpy(&lIeee80211Header.addr1[0],
           aData.substr(Net_8023_Constants::cDestinationAddressIndex, Net_8023_Constants::cDestinationAddressLength)
               .data(),
           Net_80211_Constants::cDestinationAddressLength * sizeof(uint8_t));

    memcpy(&lIeee80211Header.addr2[0],
           aData.substr(Net_8023_Constants::cSourceAddressIndex, Net_8023_Constants::cSourceAddressLength).data(),
           Net_80211_Constants::cSourceAddressLength * sizeof(uint8_t));

    uint32_t lBSSID = MacToInt(aBSSID);
    memcpy(&lIeee80211Header.addr3[0], &lBSSID, sizeof(uint32_t));

    memcpy(aPacket + aPacketIndex, &lIeee80211Header, sizeof(lIeee80211Header));
}

std::string PacketConverter::ConvertPacketTo80211(std::string_view aData, std::string_view aBSSID)
{
    std::string lReturn;
    if (aData.size() > Net_8023_Constants::cHeaderLength) {
        unsigned int lRadioTapHeaderSize{sizeof(RadioTapHeader) + sizeof(RadioTap_Constants::cFlags) +
                                         sizeof(RadioTap_Constants::cChannel) +
                                         sizeof(RadioTap_Constants::cChannelFlags) +
                                         sizeof(RadioTap_Constants::cRateFlags) + sizeof(RadioTap_Constants::cTXFlags)};
        unsigned int lIeee80211HeaderSize{sizeof(ieee80211_hdr)};
        unsigned int lLLCHeaderSize{sizeof(uint64_t)};
        unsigned int lDataSize{
            static_cast<unsigned int>(aData.size() - (Net_8023_Constants::cHeaderLength * sizeof(char)))};

        std::vector<char> lFullPacket;
        lFullPacket.reserve(lRadioTapHeaderSize + lIeee80211HeaderSize + lLLCHeaderSize + lDataSize);
        lFullPacket.resize(lRadioTapHeaderSize + lIeee80211HeaderSize + lLLCHeaderSize + lDataSize);

        // RadioTap Header
        InsertRadioTapHeader(aData, &lFullPacket[0]);

        // IEEE80211 Header
        InsertIeee80211Header(aData, aBSSID, &lFullPacket[0], lRadioTapHeaderSize);

        // Logical Link Control (LLC) header
        uint64_t lLLC = Net_80211_Constants::cSnapLLC;

        // Set EtherType from ethernet frame
        uint64_t lEtherType = *reinterpret_cast<const uint16_t*>(
            aData.substr(Net_8023_Constants::cEtherTypeIndex, Net_8023_Constants::cEtherTypeLength).data());

        lLLC |= lEtherType << 48LLU;

        memcpy(&lFullPacket[0] + lRadioTapHeaderSize + lIeee80211HeaderSize, &lLLC, sizeof(lLLC));

        // Data, without header included
        memcpy(&lFullPacket[0] + lRadioTapHeaderSize + lIeee80211HeaderSize + lLLCHeaderSize,
               aData.data() + (Net_8023_Constants::cHeaderLength * (sizeof(char))),
               aData.size() - (Net_8023_Constants::cHeaderLength * (sizeof(char))));

        lReturn = std::string(lFullPacket.begin(), lFullPacket.end());
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet", Logger::WARNING);
    }

    return lReturn;
}
