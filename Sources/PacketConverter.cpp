#ifdef _MSC_VER
#include <stdlib.h>
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)
#else

#include <byteswap.h>  // bswap_16 bswap_32 bswap_64

#endif

#include "../Includes/Logger.h"
#include "../Includes/PacketConverter.h"

#include <numeric>
#include <regex>

// Skip use of ether_aton because that could hinder Windows support.
uint64_t MacToInt(std::string_view aMac)
{
    std::istringstream lStringStream(aMac.data());
    uint64_t lNibble;
    uint64_t lResult(0);
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
        mIndexAfterRadioTap = *reinterpret_cast<const uint16_t*>(aData.data() + cRadioTapLengthIndex);
    }
}

bool PacketConverter::Is80211Data(std::string_view aData)
{
    bool lReturn{false};
    UpdateIndexAfterRadioTap(aData);

    // Sometimes it seems to send malformed packets.
    if (mIndexAfterRadioTap <= 255) {
        lReturn = *reinterpret_cast<const uint8_t*>(aData.data() + mIndexAfterRadioTap) == c80211DataType;
    } else {
        lReturn = false;
    }
    return lReturn;
}

bool PacketConverter::IsForBSSID(std::string_view aData, std::string_view aBSSID)
{
    UpdateIndexAfterRadioTap(aData);

    uint64_t lMac = *reinterpret_cast<const uint64_t*>(aData.data() + mIndexAfterRadioTap + cBSSIDIndex);
    lMac &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48u) - 1); // it's actually a uint48.

    // Big- to Little endian
    lMac = bswap_64(lMac);
    lMac = lMac >> 16u;

    return lMac == MacToInt(aBSSID);
}

std::string PacketConverter::ConvertPacketToPromiscuous(std::string_view aData)
{
    UpdateIndexAfterRadioTap(aData);

    std::string lConvertedPacket{};

    // The header should have it's complete size for the packet to be valid.
    if (aData.size() > cHeaderLength + mIndexAfterRadioTap) {
        lConvertedPacket.reserve(aData.size() - cDataIndex - mIndexAfterRadioTap - 1);
        lConvertedPacket.append(
                aData.substr(cDestinationAddressIndex + mIndexAfterRadioTap, cDestinationAddressLength));
        lConvertedPacket.append(aData.substr(cSourceAddressIndex + mIndexAfterRadioTap, cSourceAddressLength));
        lConvertedPacket.append(aData.substr(cTypeIndex + mIndexAfterRadioTap, cTypeLength));
        lConvertedPacket.append(aData.substr(cDataIndex + mIndexAfterRadioTap, aData.size() - cDataIndex - 1));
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet", Logger::WARNING);
    }

    // [ Destination MAC | Source MAC | EtherType ] [ Payload ]
    return lConvertedPacket;
}
