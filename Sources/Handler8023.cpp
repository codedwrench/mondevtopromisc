#include "../Includes/Handler8023.h"

/* Copyright (c) 2020 [Rick de Bondt] - Handler8023.cpp */

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

void Handler8023::AddToMACBlackList(uint64_t aMAC)
{
    if (!IsMACBlackListed(aMAC)) {
        Logger::GetInstance().Log("Added: " + IntToMac(aMAC) + " to blacklist.", Logger::Level::TRACE);
        mBlackList.push_back(aMAC);
    }
}

void Handler8023::AddToMACWhiteList(uint64_t aMAC)
{
    Logger::GetInstance().Log("Added: " + IntToMac(aMAC) + " to whitelist.", Logger::Level::TRACE);
    mWhiteList.push_back(aMAC);
}

void Handler8023::ClearMACBlackList()
{
    mBlackList.clear();
}

void Handler8023::ClearMACWhiteList()
{
    mWhiteList.clear();
}

std::string Handler8023::ConvertPacket(uint64_t aBSSID, RadioTapReader::PhysicalDeviceParameters aParameters)
{
    std::string lReturn;
    if (mLastReceivedData.size() > Net_8023_Constants::cHeaderLength) {
        unsigned int lIeee80211HeaderSize{sizeof(ieee80211_hdr)};
        unsigned int lLLCHeaderSize{sizeof(uint64_t)};
        unsigned int lDataSize{
            static_cast<unsigned int>(mLastReceivedData.size() - (Net_8023_Constants::cHeaderLength * sizeof(char)))};

        unsigned int lReserveSize{lIeee80211HeaderSize + lLLCHeaderSize + lDataSize};

        std::vector<char> lFullPacket;
        lFullPacket.reserve(lReserveSize);
        lFullPacket.resize(lReserveSize);

        unsigned int lIndex{0};

        // RadioTap Header
        int lRadioTapSize = InsertRadioTapHeader(&lFullPacket[0], aParameters);
        lIndex += lRadioTapSize;

        lFullPacket.reserve(lReserveSize + lRadioTapSize);
        lFullPacket.resize(lReserveSize + lRadioTapSize);

        // IEEE80211 Header
        InsertIEEE80211Header(
            &lFullPacket[0], SwapMacEndian(mSourceMAC), SwapMacEndian(mDestinationMAC), aBSSID, lIndex);
        lIndex += lIeee80211HeaderSize;

        // Logical Link Control (LLC) header
        uint64_t lLLC = Net_80211_Constants::cSnapLLC;

        // Set EtherType from ethernet frame
        uint64_t lEtherType =
            *reinterpret_cast<const uint16_t*>(&mLastReceivedData.at(Net_8023_Constants::cEtherTypeIndex));

        lLLC |= lEtherType << 48LLU;

        memcpy(&lFullPacket[0] + lIndex, &lLLC, sizeof(lLLC));
        lIndex += lLLCHeaderSize;

        // Data, without header included
        memcpy(&lFullPacket[0] + lIndex,
               mLastReceivedData.data() + (Net_8023_Constants::cHeaderLength * (sizeof(char))),
               mLastReceivedData.size() - (Net_8023_Constants::cHeaderLength * (sizeof(char))));

        lReturn = std::string(lFullPacket.begin(), lFullPacket.end());
    } else {
        Logger::GetInstance().Log("The header has an invalid length, cannot convert the packet",
                                  Logger::Level::WARNING);
    }

    return lReturn;
}

uint64_t Handler8023::GetDestinationMAC() const
{
    return mDestinationMAC;
}

std::string_view Handler8023::GetPacket()
{
    return mLastReceivedData;
}

uint64_t Handler8023::GetSourceMAC() const
{
    return mSourceMAC;
}


bool Handler8023::IsMACAllowed(uint64_t aMAC)
{
    bool lReturn{false};

    if (mWhiteList.empty()) {
        if (std::find(mBlackList.begin(), mBlackList.end(), aMAC) == mBlackList.end()) {
            lReturn = true;
        }
    } else {
        if (std::find(mWhiteList.begin(), mWhiteList.end(), aMAC) != mWhiteList.end()) {
            lReturn = true;
        }
    }

    return lReturn;
}

bool Handler8023::IsMACBlackListed(uint64_t aMAC) const
{
    bool lReturn{false};

    if (std::find(mBlackList.begin(), mBlackList.end(), aMAC) != mBlackList.end()) {
        lReturn = true;
    }

    return lReturn;
}

void Handler8023::Update(std::string_view aPacket)
{
    // Save data in object and fill RadioTap parameters.
    mLastReceivedData = aPacket;

    auto lSourceMAC = GetRawData<uint64_t>(mLastReceivedData, Net_8023_Constants::cSourceAddressIndex);
    lSourceMAC &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48U) - 1);  // it's actually a uint48.

    // Big- to Little endian
    mSourceMAC = SwapMacEndian(lSourceMAC);

    auto lDestinationMAC = GetRawData<uint64_t>(mLastReceivedData, Net_8023_Constants::cDestinationAddressIndex);
    lDestinationMAC &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48U) - 1);  // it's actually a uint48.

    // Big- to Little endian
    mDestinationMAC = SwapMacEndian(lDestinationMAC);
}
