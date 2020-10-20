#include "../Includes/PacketConverter.h"

/* Copyright (c) 2020 [Rick de Bondt] - PacketConverter.cpp */

#if defined(_MSC_VER) or defined(__MINGW32__)
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

// Helper function to get raw data more easily
template<typename Type> Type GetRawData(std::string_view aData, unsigned int aIndex)
{
    return (*reinterpret_cast<const Type*>(aData.data() + aIndex));
}

std::string GetRawString(std::string_view aData, unsigned int aIndex, unsigned int aLength)
{
    const char* lData{reinterpret_cast<const char*>(aData.data() + aIndex)};
    return std::string(lData, aLength);
}

void PacketConverter::Update(std::string_view aData)
{
    // TODO: Preload more data, like packet type and MAC addresses
    if (mRadioTap) {
        mRadioTapReader.FillRadioTapParameters(aData);
    } else {
        // If no radiotap present, reset parameters to default
        mRadioTapReader.Reset();
    }
}

// Skip use of ether_aton because that could hinder Windows support
uint64_t PacketConverter::MacToInt(std::string_view aMac)
{
    uint64_t lResult{0};

    if (!aMac.empty()) {
        std::istringstream lStringStream(aMac.data());
        uint64_t           lNibble{0};
        lStringStream >> std::hex;
        while (lStringStream >> lNibble) {
            lResult = (lResult << 8) + lNibble;
            lStringStream.get();
        }
    }

    return lResult;
}

int PacketConverter::ConvertChannelToFrequency(int aChannel)
{
    int lReturn{-1};

    // 2.4GHz, steps of 5hz.
    if (aChannel >= 1 && aChannel <= 13) {
        lReturn = 2412 + ((aChannel - 1) * 5);
    }

    return lReturn;
}

PacketConverter::PacketConverter(bool aRadioTap)
{
    mRadioTap = aRadioTap;
}

bool PacketConverter::Is80211Beacon(std::string_view aData)
{
    bool lReturn{GetRawData<uint16_t>(aData, mRadioTapReader.GetLength()) == Net_80211_Constants::cBeaconType};

    return lReturn;
}

std::string PacketConverter::GetBeaconSSID(std::string_view aData)
{
    std::string lReturn{};

    uint8_t lSSIDLength{GetRawData<uint8_t>(
        aData, mRadioTapReader.GetLength() + Net_80211_Constants::cFixedParameterTypeSSIDIndex + 1)};
    lReturn = GetRawString(
        aData, mRadioTapReader.GetLength() + Net_80211_Constants::cFixedParameterTypeSSIDIndex + 2, lSSIDLength);

    return lReturn;
}

uint64_t PacketConverter::GetBSSID(std::string_view aData)
{
    uint64_t lBSSID{0};
    lBSSID = GetRawData<uint64_t>(aData, mRadioTapReader.GetLength() + Net_80211_Constants::cBSSIDIndex);
    lBSSID &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48U) - 1);  // it's actually a uint48.

    // Big- to Little endian
    lBSSID = SwapMacEndian(lBSSID);

    return lBSSID;
}

int PacketConverter::FillSSID(std::string_view                              aData,
                              IPCapDevice_Constants::WiFiBeaconInformation& aWifiInfo,
                              uint64_t                                      aIndex)
{
    std::string lSSID = GetBeaconSSID(aData);
    aWifiInfo.SSID    = lSSID;

    return lSSID.length();
}


int FillMaxRate(std::string_view aData, IPCapDevice_Constants::WiFiBeaconInformation& aWifiInfo, uint64_t aIndex)
{
    auto lMaxRateLength = GetRawData<uint8_t>(aData, aIndex);

    // Go to index, skip past length info, then go past the highest rate and down 1 byte again so we get to the actual
    // rate
    auto lMaxRate = GetRawData<uint8_t>(aData, aIndex + 1 + lMaxRateLength - 1);

    if (aWifiInfo.MaxRate < lMaxRate) {
        aWifiInfo.MaxRate = lMaxRate;
    }

    return lMaxRateLength;
}

int FillChannelInfo(std::string_view aData, IPCapDevice_Constants::WiFiBeaconInformation& aWifiInfo, uint64_t aIndex)
{
    // Don't need to know the size for channel, so just grab the channel immediately
    auto lChannel = GetRawData<uint8_t>(aData, aIndex + 1);

    aWifiInfo.Frequency = PacketConverter::ConvertChannelToFrequency(lChannel);

    return 1;
}

bool PacketConverter::FillWiFiInformation(std::string_view                              aData,
                                          IPCapDevice_Constants::WiFiBeaconInformation& aWifiInfo)
{
    bool lReturn{false};

    // If there is an FCS remove 4 bytes from total length
    unsigned int lFCSLength = ((mRadioTapReader.GetFlags() & RadioTap_Constants::cFCSAvailableFlag) != 0) ? 4 : 0;

    // Add the BSSID
    aWifiInfo.BSSID = GetBSSID(aData);

    // First parameter is always SSID
    unsigned long lIndex{static_cast<unsigned long>(mRadioTapReader.GetLength() +
                                                    Net_80211_Constants::cFixedParameterTypeSSIDIndex + 1)};
    int           lParameterLength{FillSSID(aData, aWifiInfo, lIndex)};

    if (lParameterLength > 0) {
        // Then go fill out all the others, always adding +1 to skip past the type info
        lIndex = lIndex + lParameterLength + 1;
        while (lIndex < (aData.length()) - lFCSLength) {
            auto lParameterType = GetRawData<uint8_t>(aData, lIndex);
            switch (lParameterType) {
                case Net_80211_Constants::cFixedParameterTypeSupportedRates:
                    lIndex += FillMaxRate(aData, aWifiInfo, lIndex + 1) + 2;
                    break;
                case Net_80211_Constants::cFixedParameterTypeDSParameterSet:
                    lIndex += FillChannelInfo(aData, aWifiInfo, lIndex + 1) + 2;
                    break;
                case Net_80211_Constants::cFixedParameterTypeExtendedRates:
                    lIndex += FillMaxRate(aData, aWifiInfo, lIndex + 1) + 2;
                    break;
                default:
                    // Skip past unsupported parameters
                    // Skip past type, always add at least one so this loop never becomes an infinite one
                    lIndex += 1;
                    lIndex += GetRawData<uint8_t>(aData, lIndex) + 1;
                    break;
            }
        }
    }

    return lReturn;
}

bool PacketConverter::Is80211Data(std::string_view aData)
{
    // Do not care about subtype!
    return (GetRawData<uint8_t>(aData, mRadioTapReader.GetLength()) & 0x0FU) == Net_80211_Constants::cDataType;
}

bool PacketConverter::Is80211QOS(std::string_view aData)
{
    return GetRawData<uint8_t>(aData, mRadioTapReader.GetLength()) == Net_80211_Constants::cDataQOSType;
}

bool PacketConverter::Is80211QOSRetry(std::string_view aData)
{
    return GetRawData<uint8_t>(aData, mRadioTapReader.GetLength()) + 1 == Net_80211_Constants::cDataQOSRetryFlag;
}

bool PacketConverter::Is80211NullFunc(std::string_view aData)
{
    return GetRawData<uint8_t>(aData, mRadioTapReader.GetLength()) == Net_80211_Constants::cDataNullFuncType;
}

bool PacketConverter::IsForBSSID(std::string_view aData, uint64_t aBSSID)
{
    return aBSSID == GetBSSID(aData);
}

bool PacketConverter::IsFromMac(std::string_view aData, uint64_t aMac)
{
    return aMac == GetSourceMac(aData);
}

uint64_t PacketConverter::GetSourceMac(std::string_view aData)
{
    uint64_t lSourceMac{
        GetRawData<uint64_t>(aData, mRadioTapReader.GetLength() + Net_80211_Constants::cSourceAddressIndex)};
    lSourceMac &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48U) - 1);  // it's actually a uint48.

    // Big- to Little endian
    lSourceMac = bswap_64(lSourceMac);
    lSourceMac = lSourceMac >> 16U;

    return lSourceMac;
}

uint64_t PacketConverter::GetDestinationMac(std::string_view aData)
{
    uint64_t lDestinationMac{
        GetRawData<uint64_t>(aData, mRadioTapReader.GetLength() + Net_80211_Constants::cDestinationAddressIndex)};
    lDestinationMac &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48U) - 1);  // it's actually a uint48.

    // Big- to Little endian
    lDestinationMac = bswap_64(lDestinationMac);
    lDestinationMac = lDestinationMac >> 16U;

    return lDestinationMac;
}

std::string PacketConverter::ConvertPacketTo8023(std::string_view aData)
{
    std::string lConvertedPacket{};

    unsigned int lFCSLength = ((mRadioTapReader.GetFlags() & RadioTap_Constants::cFCSAvailableFlag) != 0) ? 4 : 0;

    unsigned int lSourceAddressIndex      = Net_80211_Constants::cSourceAddressIndex + mRadioTapReader.GetLength();
    unsigned int lDestinationAddressIndex = Net_80211_Constants::cDestinationAddressIndex + mRadioTapReader.GetLength();
    unsigned int lTypeIndex               = Net_80211_Constants::cEtherTypeIndex + mRadioTapReader.GetLength();
    unsigned int lDataIndex               = Net_80211_Constants::cDataIndex + mRadioTapReader.GetLength();

    // If there is QOS data added to the 80211 header, we need to skip past that as well
    if (Is80211QOS(aData)) {
        lTypeIndex += sizeof(uint8_t) * Net_80211_Constants::cDataQOSLength;
        lDataIndex += sizeof(uint8_t) * Net_80211_Constants::cDataQOSLength;
    }

    // Null functions not supported.
    if (!Is80211NullFunc(aData)) {
        // The header should have it's complete size for the packet to be valid.
        if (aData.size() > Net_80211_Constants::cDataHeaderLength + mRadioTapReader.GetLength()) {
            // Strip framecheck sequence as well.
            lConvertedPacket.reserve(aData.size() - Net_80211_Constants::cDataIndex - mRadioTapReader.GetLength() -
                                     lFCSLength);

            lConvertedPacket.append(
                aData.substr(lDestinationAddressIndex, Net_80211_Constants::cDestinationAddressLength));

            lConvertedPacket.append(aData.substr(lSourceAddressIndex, Net_80211_Constants::cSourceAddressLength));

            lConvertedPacket.append(aData.substr(lTypeIndex, Net_80211_Constants::cEtherTypeLength));

            lConvertedPacket.append(aData.substr(lDataIndex, aData.size() - lDataIndex - lFCSLength));
        } else {
            Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet",
                                      Logger::Level::WARNING);
        }
    }
    // [ Destination MAC | Source MAC | EtherType ] [ Payload ]
    return lConvertedPacket;
}

// Helper function for ConvertPacketTo80211, adds the radiotap header.
void PacketConverter::InsertRadioTapHeader(char* aPacket, uint16_t aFrequency, uint8_t aMaxRate) const
{
    unsigned int lIndex{sizeof(RadioTapHeader)};

    // RadioTap Header
    RadioTapHeader lRadioTapHeader{};
    memset(&lRadioTapHeader, 0, sizeof(lRadioTapHeader));

    // General header
    lRadioTapHeader.present_flags   = RadioTap_Constants::cSendPresentFlags;
    lRadioTapHeader.bytes_in_header = RadioTap_Constants::cRadioTapSize;

    memcpy(aPacket, &lRadioTapHeader, sizeof(lRadioTapHeader));

    // Optional header (Flags)
    uint8_t lFlags{RadioTap_Constants::cFlags};
    memcpy(aPacket + lIndex, &lFlags, sizeof(lFlags));
    lIndex += sizeof(lFlags);

    // Optional header (Rate Flags)
    uint8_t lRateFlags{aMaxRate};
    memcpy(aPacket + lIndex, &lRateFlags, sizeof(lRateFlags));
    lIndex += sizeof(lRateFlags);

    // Optional headers (Channel & Channel Flags)
    uint16_t lChannel{aFrequency};
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
void InsertIeee80211Header(std::string_view aData, uint64_t aBSSID, char* aPacket, unsigned int aPacketIndex)
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

    uint64_t lBSSID = aBSSID;

    // Little- to Big endian
    lBSSID = bswap_64(lBSSID);
    lBSSID = lBSSID >> 16U;
    memcpy(&lIeee80211Header.addr3[0], &lBSSID, Net_80211_Constants::cBSSIDLength * sizeof(uint8_t));

    memcpy(aPacket + aPacketIndex, &lIeee80211Header, sizeof(lIeee80211Header));
}

std::string PacketConverter::ConvertPacketTo80211(std::string_view aData,
                                                  uint64_t         aBSSID,
                                                  uint16_t         aFrequency,
                                                  uint8_t          aMaxRate)
{
    std::string lReturn;
    if (aData.size() > Net_8023_Constants::cHeaderLength) {
        unsigned int lIeee80211HeaderSize{sizeof(ieee80211_hdr)};
        unsigned int lLLCHeaderSize{sizeof(uint64_t)};
        unsigned int lDataSize{
            static_cast<unsigned int>(aData.size() - (Net_8023_Constants::cHeaderLength * sizeof(char)))};

        unsigned int lReserveSize{lIeee80211HeaderSize + lLLCHeaderSize + lDataSize};
        if (mRadioTap) {
            lReserveSize += RadioTap_Constants::cRadioTapSize;
        }

        std::vector<char> lFullPacket;
        lFullPacket.reserve(lReserveSize);
        lFullPacket.resize(lReserveSize);

        unsigned int lIndex{0};

        if (mRadioTap) {
            // RadioTap Header
            InsertRadioTapHeader(&lFullPacket[0], aFrequency, aMaxRate);
            lIndex += RadioTap_Constants::cRadioTapSize;
        }

        // IEEE80211 Header
        InsertIeee80211Header(aData, aBSSID, &lFullPacket[0], lIndex);
        lIndex += lIeee80211HeaderSize;

        // Logical Link Control (LLC) header
        uint64_t lLLC = Net_80211_Constants::cSnapLLC;

        // Set EtherType from ethernet frame
        uint64_t lEtherType = *reinterpret_cast<const uint16_t*>(
            aData.substr(Net_8023_Constants::cEtherTypeIndex, Net_8023_Constants::cEtherTypeLength).data());

        lLLC |= lEtherType << 48LLU;

        memcpy(&lFullPacket[0] + lIndex, &lLLC, sizeof(lLLC));
        lIndex += lLLCHeaderSize;

        // Data, without header included
        memcpy(&lFullPacket[0] + lIndex,
               aData.data() + (Net_8023_Constants::cHeaderLength * (sizeof(char))),
               aData.size() - (Net_8023_Constants::cHeaderLength * (sizeof(char))));

        lReturn = std::string(lFullPacket.begin(), lFullPacket.end());
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet",
                                  Logger::Level::WARNING);
    }

    return lReturn;
}

// Helper function for ConstructAcknowledgementFrame, adds the acknowledgement frame
void InsertAcknowledgementFrame(std::array<uint8_t, 6> aReceiverMac, char* aPacket, unsigned int aPacketIndex)
{
    AcknowledgementHeader lAcknowledgementHeader{};
    memset(&lAcknowledgementHeader, 0, sizeof(lAcknowledgementHeader));

    lAcknowledgementHeader.frame_control = Net_80211_Constants::cAcknowledgementType;
    lAcknowledgementHeader.duration_id   = 0xffff;  // Just an arbitrarily high number.

    memcpy(&lAcknowledgementHeader.recv_address[0],
           aReceiverMac.data(),
           Net_80211_Constants::cDestinationAddressLength * sizeof(uint8_t));

    memcpy(aPacket + aPacketIndex, &lAcknowledgementHeader, sizeof(lAcknowledgementHeader));
}

std::string PacketConverter::ConstructAcknowledgementFrame(std::array<uint8_t, 6> aReceiverMac,
                                                           uint16_t               aFrequency,
                                                           uint8_t                aMaxRate)
{
    std::string  lReturn;
    unsigned int lReserveSize{sizeof(AcknowledgementHeader)};
    if (mRadioTap) {
        lReserveSize += RadioTap_Constants::cRadioTapSize;
    }

    std::vector<char> lFullPacket;
    lFullPacket.reserve(lReserveSize);
    lFullPacket.resize(lReserveSize);

    unsigned int lIndex{0};

    if (mRadioTap) {
        // RadioTap Header
        InsertRadioTapHeader(&lFullPacket[0], aFrequency, aMaxRate);
        lIndex += RadioTap_Constants::cRadioTapSize;
    }

    InsertAcknowledgementFrame(aReceiverMac, lFullPacket.data(), lIndex);

    lReturn = std::string(lFullPacket.begin(), lFullPacket.end());
    return lReturn;
}

void PacketConverter::SetRadioTap(bool aRadioTap)
{
    mRadioTap = aRadioTap;
}

uint64_t PacketConverter::SwapMacEndian(uint64_t aMac) {
    // Little- to Big endian
    aMac = bswap_64(aMac);
    return aMac >> 16U;
}