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
        lConvertedPacket.reserve(aData.size() - Net_80211_Constants::cDataIndex - mIndexAfterRadioTap - 1);

        lConvertedPacket.append(aData.substr(Net_80211_Constants::cDestinationAddressIndex + mIndexAfterRadioTap,
                                             Net_80211_Constants::cDestinationAddressLength));

        lConvertedPacket.append(aData.substr(Net_80211_Constants::cSourceAddressIndex + mIndexAfterRadioTap,
                                             Net_80211_Constants::cSourceAddressLength));

        lConvertedPacket.append(
            aData.substr(Net_80211_Constants::cTypeIndex + mIndexAfterRadioTap, Net_80211_Constants::cTypeLength));

        lConvertedPacket.append(aData.substr(Net_80211_Constants::cDataIndex + mIndexAfterRadioTap,
                                             aData.size() - Net_80211_Constants::cDataIndex - 1));
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet", Logger::WARNING);
    }

    // [ Destination MAC | Source MAC | EtherType ] [ Payload ]
    return lConvertedPacket;
}

// TODO: Definitely rewrite
std::string PacketConverter::ConvertPacketTo80211(std::string_view aData, std::string_view aBSSID)
{
    if (aData.size() > Net_8023_Constants::cHeaderLength) {
        unsigned int lRadioTapHeaderSize{sizeof(RadioTapHeader) + sizeof(RadioTap_Constants::cTXFlags)};
        unsigned int lIeee80211HeaderSize{sizeof(ieee80211_hdr)};

        RadioTapHeader lRadioTapHeader{};
        memset(&lRadioTapHeader, 0, sizeof(lRadioTapHeader));

        // General header + our optional field.
        lRadioTapHeader.present_flags   = RadioTap_Constants::cSendPresentFlags;
        lRadioTapHeader.bytes_in_header = lRadioTapHeaderSize;

        std::array<uint8_t, sizeof(lRadioTapHeader)> lRadioTapHeaderData{};
        memcpy(lRadioTapHeaderData.data(), &lRadioTapHeader, sizeof(lRadioTapHeader));

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

        std::array<uint8_t, sizeof(ieee80211_hdr)> lIeee80211HeaderData{};
        memcpy(lIeee80211HeaderData.data(), &lIeee80211Header, sizeof(lIeee80211HeaderData));

        // Logical Link Control (LLC) header
        uint64_t lLLC = Net_80211_Constants::cSnapLLC;

        // Set EtherType from ethernet frame
        lLLC += *reinterpret_cast<const uint64_t*>(
            aData.substr(Net_8023_Constants::cEtherTypeIndex, Net_8023_Constants::cEtherTypeLength).data());

        // Data
        std::string_view lData =
            aData.substr(Net_8023_Constants::cEtherDataIndex, aData.size() - Net_8023_Constants::cEtherDataIndex - 1);


        unsigned int lTotalPacketSize{static_cast<unsigned int>(lRadioTapHeaderSize + lIeee80211HeaderSize +
                                                                sizeof(Net_80211_Constants::cSnapLLC) + lData.size())};

        std::string lFullPacket{};
        lFullPacket.reserve(lTotalPacketSize);

        lFullPacket.append(std::string(lRadioTapHeaderData.begin(), lRadioTapHeaderData.end()));

        std::array<uint8_t, sizeof(RadioTap_Constants::cTXFlags)> lTXFlags{};
        memcpy(&lTXFlags, &RadioTap_Constants::cTXFlags, sizeof(RadioTap_Constants::cTXFlags));
        lFullPacket.append(std::string(lTXFlags.begin(), lTXFlags.end()));

        lFullPacket.append(lIeee80211HeaderData.begin(), lIeee80211HeaderData.end());

        std::array<uint8_t, sizeof(lLLC)> lLLCData{};
        memcpy(&lLLCData, &lLLC, sizeof(lLLC));

        lFullPacket.append(lLLCData.begin(), lLLCData.end());

        lFullPacket.append(lData.data());

        return lFullPacket;
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet", Logger::WARNING);
    }

    return {};
}
