#include "../Includes/Parameter80211Reader.h"

#include <utility>

#include <sys/socket.h>

/* Copyright (c) 2020 [Rick de Bondt] - Parameter80211Reader.cpp */

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/NetworkingHeaders.h"

Parameter80211Reader::Parameter80211Reader(std::shared_ptr<RadioTapReader> aPhysicalDeviceHeaderReader)
{
    mPhysicalDeviceHeaderReader = std::move(aPhysicalDeviceHeaderReader);
}

uint8_t Parameter80211Reader::GetFrequency() const
{
    return mFrequency;
}

uint8_t Parameter80211Reader::GetMaxRate() const
{
    return mMaxRate;
}

std::string_view Parameter80211Reader::GetSSID() const
{
    return mSSID;
}

bool Parameter80211Reader::GetIsAdhoc() const
{
    return mIsAdhoc;
}

void Parameter80211Reader::Update(std::string_view aData)
{
    mLastReceivedPacket = aData;

    // Re-obtain this
    mMaxRate = 0;
    mIsAdhoc = false;

    // If there is an FCS remove 4 bytes from total length
    unsigned int lFCSLength =
        ((mPhysicalDeviceHeaderReader != nullptr) &&
         ((mPhysicalDeviceHeaderReader->GetFlags() & RadioTap_Constants::cFCSAvailableFlag) != 0)) ?
            4 :
            0;

    // First parameter is always SSID
    uint16_t lIndex{};

    if (mPhysicalDeviceHeaderReader != nullptr) {
        lIndex += mPhysicalDeviceHeaderReader->GetLength();
    }
    lIndex += static_cast<uint16_t>(Net_80211_Constants::cFixedParameterTypeSSIDIndex + 1);

    uint8_t lParameterLength{UpdateSSID(lIndex)};
    lIndex++;

    if (lParameterLength > 0) {
        // Then go fill out all the others, always adding +1 to skip past the type info
        lIndex = lIndex + lParameterLength + 1;
        while (lIndex < (aData.length()) - lFCSLength) {
            auto lParameterType = GetRawData<uint8_t>(aData, lIndex);
            switch (lParameterType) {
                case Net_80211_Constants::cFixedParameterTypeSupportedRates:
                    lIndex += UpdateMaxRate(lIndex + 1) + 2;
                    break;
                case Net_80211_Constants::cFixedParameterTypeDSParameterSet:
                    lIndex += UpdateChannelInfo(lIndex + 1) + 2;
                    break;
                case Net_80211_Constants::cFixedParameterTypeExtendedRates:
                    lIndex += UpdateMaxRate(lIndex + 1) + 2;
                    break;
                case Net_80211_Constants::cFixedParameterTypeIBSS:
                    lIndex += UpdateIsAdhoc(lIndex + 1) + 2;
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
}

uint8_t Parameter80211Reader::UpdateChannelInfo(uint16_t aIndex)
{
    // Don't need to know the size for channel, so just grab the channel immediately
    auto lChannel = GetRawData<uint8_t>(mLastReceivedPacket, aIndex + 1);

    mFrequency = ConvertChannelToFrequency(lChannel);

    return 1;
}

uint8_t Parameter80211Reader::UpdateMaxRate(uint16_t aIndex)
{
    auto lMaxRateLength = GetRawData<uint8_t>(mLastReceivedPacket, aIndex);

    // Go to index, skip past length info, then go past the highest rate and down 1 byte again so we get to the actual
    // rate
    auto lMaxRate = GetRawData<uint8_t>(mLastReceivedPacket, aIndex + 1 + lMaxRateLength - 1);

    if (mMaxRate < lMaxRate) {
        mMaxRate = lMaxRate;
    }

    return lMaxRateLength;
}

uint8_t Parameter80211Reader::UpdateIsAdhoc(uint16_t aIndex)
{
    auto lIBSSLength = GetRawData<uint8_t>(mLastReceivedPacket, aIndex);
    mIsAdhoc         = true;

    return lIBSSLength;
}

uint8_t Parameter80211Reader::UpdateSSID(uint16_t aIndex)
{
    uint8_t lSSIDLength{0};
    lSSIDLength = GetRawData<uint8_t>(mLastReceivedPacket, aIndex);
    mSSID       = GetRawString(mLastReceivedPacket, aIndex + 1, lSSIDLength);

    return lSSIDLength;
}

void Parameter80211Reader::Reset()
{
    mFrequency = 0;
    mMaxRate   = 0;
    mSSID      = "";
}
