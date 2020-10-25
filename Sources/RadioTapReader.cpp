#include "../Includes/RadioTapReader.h"

/* Copyright (c) 2020 [Rick de Bondt] - RadioTapReader.cpp */


// Helper function to get raw data more easily
template<typename Type> Type GetRawData(std::string_view aData, unsigned int aIndex)
{
    return (*reinterpret_cast<const Type*>(aData.data() + aIndex));
}

RadioTapReader::PhysicalDeviceParameters RadioTapReader::ExportRadioTapParameters()
{
    return mParameters;
}

void RadioTapReader::FillRadioTapParameters(std::string_view aData)
{
    // Skip 2 bytes to skip header revision and header pad
    auto lLength = GetRawData<uint16_t>(aData, RadioTap_Constants::cLengthIndex);

    if (lLength <= RadioTap_Constants::cMaxLength) {
        // Valid length, we can start saving parameters
        mParameters.mLength = lLength;

        // What fields do we have?
        mParameters.mPresentFlags = GetRawData<uint32_t>(aData, RadioTap_Constants::cPresentFlagsIndex);

        uint8_t lIndex{RadioTap_Constants::cPresentFlagsIndex};

        // If extended radiotap, skip past it
        while ((GetRawData<uint32_t>(aData, lIndex) & 0x20000000U) != 0) {
            lIndex += 4;
        }

        // And skip past the last one
        lIndex += 4;

        if ((mParameters.mPresentFlags & 1U) == 1) {
            // TSFT, don't care, skip over it
            lIndex += sizeof(uint64_t);
        }
        if ((mParameters.mPresentFlags & (1U << 1U)) != 0) {
            // Flags, contains important information like datapad and fcs at the end of a packet
            mParameters.mFlags = GetRawData<uint8_t>(aData, lIndex);
            lIndex += sizeof(uint8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 2U)) != 0) {
            // Rate
            mParameters.mDataRate = GetRawData<uint8_t>(aData, lIndex);
            lIndex += sizeof(uint8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 3U)) != 0) {
            // Channel and channel flags
            mParameters.mFrequency = GetRawData<uint16_t>(aData, lIndex);
            lIndex += sizeof(uint16_t);
            mParameters.mChannelFlags = GetRawData<uint16_t>(aData, lIndex);
        }
        if ((mParameters.mPresentFlags & (1U << 4U)) != 0) {
            // FHSS, don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mParameters.mPresentFlags & (1U << 5U)) != 0) {
            // Antenna signal, Don't care skip over it
            lIndex += sizeof(int8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 6U)) != 0) {
            // Antenna Noise, Don't care skip over it
            lIndex += sizeof(int8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 7U)) != 0) {
            // Lock Quality, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mParameters.mPresentFlags & (1U << 8U)) != 0) {
            // TX attenuation, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mParameters.mPresentFlags & (1U << 9U)) != 0) {
            // dB TX attenuation, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mParameters.mPresentFlags & (1U << 10U)) != 0) {
            // dBm TX power, Don't care skip over it
            lIndex += sizeof(int8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 11U)) != 0) {
            // Antenna, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 12U)) != 0) {
            // dB antenna signal, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 13U)) != 0) {
            // dB antenna noise, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 14U)) != 0) {
            // RX Flags, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mParameters.mPresentFlags & (1U << 15U)) != 0) {
            // TX Flags, Don't care skip over it
            lIndex += sizeof(uint16_t);
        }
        if ((mParameters.mPresentFlags & (1U << 16U)) != 0) {
            // RTS Retries Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 17U)) != 0) {
            // Data retries, Don't care skip over it
            lIndex += sizeof(uint8_t);
        }
        if ((mParameters.mPresentFlags & (1U << 18U)) != 0) {
            // XChannel, Don't care skip over it
            lIndex += sizeof(uint64_t);
        }
        if ((mParameters.mPresentFlags & (1U << 19U)) != 0) {
            // Fill MCS info
            mParameters.mKnownMCSInfo = GetRawData<uint8_t>(aData, lIndex);
            mParameters.mMCSFlags     = GetRawData<uint8_t>(aData, lIndex + 1);
            mParameters.mMCSInfo      = GetRawData<uint8_t>(aData, lIndex + 2);

            lIndex += sizeof(uint8_t) * 3;
        }

        // Don't care about any of the other flags yet, so just don't read them yet
    }
}

uint16_t RadioTapReader::GetLength() const
{
    return mParameters.mLength;
}

uint8_t RadioTapReader::GetPresentFlags() const
{
    return mParameters.mPresentFlags;
}

uint16_t RadioTapReader::GetChannelFlags() const
{
    return mParameters.mChannelFlags;
}

uint8_t RadioTapReader::GetDataRate() const
{
    return mParameters.mDataRate;
}

uint8_t RadioTapReader::GetFlags() const
{
    return mParameters.mFlags;
}

uint16_t RadioTapReader::GetFrequency() const
{
    return mParameters.mFrequency;
}

uint8_t RadioTapReader::GetKnownMCSInfo() const
{
    return mParameters.mKnownMCSInfo;
}

uint8_t RadioTapReader::GetMCSFlags() const
{
    return mParameters.mMCSFlags;
}

uint8_t RadioTapReader::GetMCSInfo() const
{
    return mParameters.mMCSInfo;
}

void RadioTapReader::Reset()
{
    mParameters.mLength       = 0;
    mParameters.mPresentFlags = RadioTap_Constants::cSendPresentFlags;
    mParameters.mFlags        = RadioTap_Constants::cFlags;
    mParameters.mDataRate     = RadioTap_Constants::cRateFlags;
    mParameters.mFrequency    = RadioTap_Constants::cChannel;
    mParameters.mChannelFlags = RadioTap_Constants::cChannelFlags;
    mParameters.mMCSFlags     = 0;
    mParameters.mKnownMCSInfo = 0;
    mParameters.mMCSInfo      = 0;
}
