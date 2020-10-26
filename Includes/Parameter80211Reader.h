#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Parameter80211Reader.h
 *
 * This file converts reads parameters from a 80211 beacon header.
 *
 **/

#include <memory>
#include <string>

#include "RadioTapReader.h"

class Parameter80211Reader
{
public:
    /**
     * Constructor for the Parameter80211Reader.
     * @param aPhysicalDeviceHeaderReader - Pointer to the device header reader, used for index after header.
     */
    explicit Parameter80211Reader(std::shared_ptr<RadioTapReader> aPhysicalDeviceHeaderReader);

    /**
     * Gets the last obtained frequency.
     * @return frequency of last updated packet, 0 if unsuccessful.
     */
    [[nodiscard]] uint8_t GetFrequency() const;

    /**
     * Gets the last obtained max data rate.
     * @return max data rate of last updated packet, 0 if unsuccessful.
     */
    [[nodiscard]] uint8_t GetMaxRate() const;

    /**
     * Gets the last obtained SSID.
     * @return SSID of last updated packet, empty string if unsuccessful.
     */
    [[nodiscard]] std::string_view GetSSID();

    /**
     * Fills parameters into this object.
     * @param aData - Packet to use to update the parameters.
     */
    void Update(std::string_view aData);

    /**
     * Resets the data in this object.
     */
    void Reset();

private:
    uint8_t UpdateChannelInfo(uint64_t aIndex);
    uint8_t UpdateMaxRate(uint64_t aIndex);
    uint8_t UpdateSSID();

    std::shared_ptr<RadioTapReader> mPhysicalDeviceHeaderReader{nullptr};

    uint8_t          mFrequency{0};
    std::string_view mLastReceivedPacket{};
    uint8_t          mMaxRate{0};
    std::string      mSSID{};
};