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
    constexpr unsigned int cTypeIndex{30};
    constexpr unsigned int cTypeLength{2};
    constexpr unsigned int cDataIndex{32};
    constexpr unsigned int cHeaderLength{cDestinationAddressLength +
                                         cSourceAddressLength +
                                         cTypeLength};
}

using namespace PacketConverter_Constants;

class PacketConverter
{
public:
    /**
     * This function converts a monitor mode packet to a promiscuous mode packet, stripping the radiotap header and
     * adding an ethernet header. Only converts data packets!
     * @param aPacketData - The packet data to convert.
     * @return converted packet data, empty string if failed.
     */
    static std::string ConvertPacketToPromiscuous(std::string_view aPacketData);
};


#endif //PACKETCONVERTER_H
