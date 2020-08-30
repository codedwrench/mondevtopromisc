#include "../Includes/Logger.h"
#include "../Includes/PacketConverter.h"

#include <string>
#include <vector>

std::string PacketConverter::ConvertPacketToPromiscuous(std::string_view aPacketData)
{
    std::string lConvertedPacket{};

    // The header should have it's complete size for the packet to be valid.
    if (aPacketData.size() > cHeaderLength) {
        lConvertedPacket.reserve(aPacketData.size() - cDataIndex - 1);
        lConvertedPacket.append(aPacketData.substr(cDestinationAddressIndex, cDestinationAddressLength));
        lConvertedPacket.append(aPacketData.substr(cSourceAddressIndex, cSourceAddressLength));
        lConvertedPacket.append(aPacketData.substr(cTypeIndex, cTypeLength));
        lConvertedPacket.append(aPacketData.substr(cDataIndex, aPacketData.size() - cDataIndex - 1));
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet", Logger::WARNING);
    }

    // [ Destination MAC | Source MAC | EtherType ] [ Payload ]
    return lConvertedPacket;
}
