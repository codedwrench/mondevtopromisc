/* Copyright (c) 2020 [Rick de Bondt] - RadioTapReader.h
 *
 * This file contains the necessary components to dissect a radiotap header and return the nessecary information.
 *
 **/

#pragma once

#include <string_view>

#include "NetworkingHeaders.h"

/**
 * This class reads the radiotap header from a packet and saves the parameters within its object.
 */
class RadioTapReader
{
public:
    /**
     * Saves parameters in this object to this struct for easy exporting.
     */
    struct PhysicalDeviceParameters
    {
        uint16_t mLength{0};
        uint32_t mPresentFlags{RadioTap_Constants::cSendPresentFlags};
        uint8_t  mFlags{RadioTap_Constants::cFlags};
        uint8_t  mDataRate{RadioTap_Constants::cRateFlags};
        uint16_t mFrequency{RadioTap_Constants::cChannel};
        uint16_t mChannelFlags{RadioTap_Constants::cChannelFlags};
        uint8_t  mKnownMCSInfo{0};
        uint8_t  mMCSFlags{0};
        uint8_t  mMCSInfo{0};
    };

    /**
     * Resets all the member variables to default.
     */
    void Reset();

    /**
     * Returns a copy of the parameters for later use.
     */
    PhysicalDeviceParameters ExportRadioTapParameters();

    /**
     * Fills this object with information about the RadioTap header, run once per packet received where the radiotap
     * header is of interest.
     * @param aData - The packet to use to fill the parameters.
     */
    void FillRadioTapParameters(std::string_view aData);

    /**
     * Get the length of the radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the length of the radiotap header.
     */
    [[nodiscard]] uint16_t GetLength() const;

    /**
     * Get the flags in the radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the flags in the radiotap header.
     */
    [[nodiscard]] uint8_t GetFlags() const;

    /**
     * Get the present flags in the radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the present flags in the radiotap header.
     */
    [[nodiscard]] uint8_t GetPresentFlags() const;

    /**
     * Get the datarate in the radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the datarate in the radiotap header.
     */
    [[nodiscard]] uint8_t GetDataRate() const;

    /**
     * Get the frequency in the radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the frequency/channel in the radiotap header.
     */
    [[nodiscard]] uint16_t GetFrequency() const;

    /**
     * Gets the channel flags in the radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the channel flags in the radiotap header.
     */
    [[nodiscard]] uint16_t GetChannelFlags() const;

    /**
     * Gets MCS flags in radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the MCS flags in the radiotap header.
     */
    [[nodiscard]] uint8_t GetMCSFlags() const;

    /**
     * Gets known MCS info in radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the known MCS info in the radiotap header.
     */
    [[nodiscard]] uint8_t GetKnownMCSInfo() const;

    /**
     * Gets MCS info in radiotap header.
     * @note Has to be called after running FillRadioTapParameters.
     * @return the MCS info in the radiotap header.
     */
    [[nodiscard]] uint8_t GetMCSInfo() const;

private:
    PhysicalDeviceParameters mParameters;
};
