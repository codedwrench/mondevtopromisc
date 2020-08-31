#include "../Includes/Logger.h"
#include "../Includes/PacketConverter.h"

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
    UpdateIndexAfterRadioTap(aData);

    return aData.at(mIndexAfterRadioTap) == c80211DataType;
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
