#include "../Includes/RadioTapReader.h"

#include "../Includes/NetworkingHeaders.h"

/* Copyright (c) 2020 [Rick de Bondt] - RadioTapReader.cpp */


// Helper function to get raw data more easily
template<typename Type> Type GetRawData(std::string_view aData, unsigned int aIndex)
{
    return (*reinterpret_cast<const Type*>(aData.data() + aIndex));
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

        uint8_t lIndex{RadioTap_Constants::cDataIndex};

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
    }
}
