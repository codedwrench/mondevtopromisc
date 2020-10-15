#include "../Includes/RadioTapReader.h"

/* Copyright (c) 2020 [Rick de Bondt] - RadioTapReader.cpp */


// Helper function to get raw data more easily
template<typename Type> Type GetRawData(std::string_view aData, unsigned int aIndex)
{
    return (*reinterpret_cast<const Type*>(aData.data() + aIndex));
}

void RadioTapReader::Reset()
{
    mLength       = 0;
    mPresentFlags = RadioTap_Constants::cSendPresentFlags;
    mFlags        = RadioTap_Constants::cFlags;
    mDataRate     = RadioTap_Constants::cRateFlags;
    mFrequency    = RadioTap_Constants::cChannel;
    mChannelFlags = RadioTap_Constants::cChannelFlags;
}

uint16_t RadioTapReader::GetLength() const
{
    return mLength;
}

uint8_t RadioTapReader::GetPresentFlags() const
{
    return mPresentFlags;
}

uint16_t RadioTapReader::GetChannelFlags() const
{
    return mChannelFlags;
}

uint8_t RadioTapReader::GetDataRate() const
{
    return mDataRate;
}

uint8_t RadioTapReader::GetFlags() const
{
    return mFlags;
}

uint16_t RadioTapReader::GetFrequency() const
{
    return mFrequency;
}

void RadioTapReader::FillRadioTapParameters(std::string_view aData)
{
    // Skip 2 bytes to skip header revision and header pad
    auto lLength = GetRawData<uint16_t>(aData, RadioTap_Constants::cLengthIndex);

    if (lLength <= RadioTap_Constants::cMaxLength) {
        // Valid length, we can start saving parameters
        mLength = lLength;

        // What fields do we have?
        mPresentFlags = GetRawData<uint32_t>(aData, RadioTap_Constants::cPresentFlagsIndex);

        uint8_t lIndex{RadioTap_Constants::cPresentFlagsIndex};

        // If extended radiotap, skip past it
        while ((GetRawData<uint32_t>(aData, lIndex) & 0x20000000U) != 0) {
            lIndex += 4;
        }

        // And skip past the last one
        lIndex += 4;

        if ((mPresentFlags & 1U) == 1) {
            // TSFT, don't care, skip over it
            lIndex += sizeof(uint64_t);
        }
        if (((mPresentFlags >> 1U) & 1U) == 1) {
            // Flags, contains important information like datapad and fcs at the end of a packet
            mFlags = GetRawData<uint8_t>(aData, lIndex);
            lIndex += sizeof(uint8_t);
        }
        if (((mPresentFlags >> 2U) & 1U) == 1) {
            // Rate
            mDataRate = GetRawData<uint8_t>(aData, lIndex);
            lIndex += sizeof(uint8_t);
        }
        if (((mPresentFlags >> 3U) & 1U) == 1) {
            // Channel and channel flags
            mFrequency = GetRawData<uint16_t>(aData, lIndex);
            lIndex += sizeof(uint16_t);
            mChannelFlags = GetRawData<uint16_t>(aData, lIndex);
        }

        // Don't care about any of the other flags yet, so just don't read them yet
    }
}
