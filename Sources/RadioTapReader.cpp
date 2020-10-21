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
    mMCSFlags     = 0;
    mKnownMCSInfo = 0;
    mMCSInfo      = 0;
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

uint8_t RadioTapReader::GetKnownMCSInfo() const
{
    return mKnownMCSInfo;
}

uint8_t RadioTapReader::GetMCSFlags() const
{
    return mMCSFlags;
}

uint8_t RadioTapReader::GetMCSInfo() const
{
    return mMCSInfo;
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
        if ((mPresentFlags & (1U << 1U)) != 0) {
            // Flags, contains important information like datapad and fcs at the end of a packet
            mFlags = GetRawData<uint8_t>(aData, lIndex);
            lIndex += sizeof(uint8_t);
        }
        if ((mPresentFlags & (1U << 2U)) != 0) {
            // Rate
            mDataRate = GetRawData<uint8_t>(aData, lIndex);
            lIndex += sizeof(uint8_t);
        }
        if ((mPresentFlags & (1U << 3U)) != 0) {
            // Channel and channel flags
            mFrequency = GetRawData<uint16_t>(aData, lIndex);
            lIndex += sizeof(uint16_t);
            mChannelFlags = GetRawData<uint16_t>(aData, lIndex);
        }
        if ((mPresentFlags & (1U << 4U)) != 0) {
            // FHSS, don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mPresentFlags & (1U << 5U)) != 0) {
            // Antenna signal, Don't care skip over it
            lIndex += sizeof(int8_t);
        }
        if ((mPresentFlags & (1U << 6U)) != 0) {
            // Antenna Noise, Don't care skip over it
            lIndex += sizeof(int8_t);
        }
        if ((mPresentFlags & (1U << 7U)) != 0) {
            // Lock Quality, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mPresentFlags & (1U << 8U)) != 0) {
            // TX attenuation, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mPresentFlags & (1U << 9U)) != 0) {
            // dB TX attenuation, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mPresentFlags & (1U << 10U)) != 0) {
            // dBm TX power, Don't care skip over it
            lIndex += sizeof(int8_t);
        }
        if ((mPresentFlags & (1U << 11U)) != 0) {
            // Antenna, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mPresentFlags & (1U << 12U)) != 0) {
            // dB antenna signal, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mPresentFlags & (1U << 13U)) != 0) {
            // dB antenna noise, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mPresentFlags & (1U << 14U)) != 0) {
            // RX Flags, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mPresentFlags & (1U << 15U)) != 0) {
            // TX Flags, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mPresentFlags & (1U << 16U)) != 0) {
            // RTS Retries Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mPresentFlags & (1U << 17U)) != 0) {
            // Data retries, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mPresentFlags & (1U << 18U)) != 0) {
            // XChannel, Don't care skip over it
            lIndex += sizeof(uint64_t);
        }
        if ((mPresentFlags & (1U << 19U)) != 0) {
            // Fill MCS info
            mKnownMCSInfo = GetRawData<uint8_t>(aData, lIndex);
            mMCSFlags     = GetRawData<uint8_t>(aData, lIndex + 1);
            mMCSInfo      = GetRawData<uint8_t>(aData, lIndex + 2);

            lIndex += sizeof(uint8_t) * 3;
        }

        // Don't care about any of the other flags yet, so just don't read them yet
    }
}
