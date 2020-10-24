#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Logger.h
 *
 * This file reads packets from a monitor format and converts to a promiscuous format.
 *
 **/

#include <memory>
#include <string>
#include <vector>

#include "NetworkingHeaders.h"
#include "RadioTapReader.h"

/**
 * This class reads packets from a monitor format and converts to a promiscuous format.
 **/
class Handler80211
{
public:
    /**
     * Constructs a handler object that converts packets from a wireless (radiotap + 802.11) format to an ethernet,
     * (802.3) format.
     * @param aType - Type of physical device header, usually radiotap is used, and for now this is the only supported
     * type.
     */
    explicit Handler80211(PhysicalDeviceHeaderType aType);

    /**
     * Clears blacklist.
     */
    void ClearMACBlackList();

    /**
     * Clears whitelist.
     */
    void ClearMACWhiteList();

    /**
     * Gets main packet type: Control/Data/Management.
     * @return packet type.
     */
    Main80211PacketType GetMainPacketType();

    /**
     * Returns the type of control frame.
     * @return control frame type.
     */
    Control80211PacketType GetControlPacketType();

    /**
     * Gets data frame type.
     * @return data frame type.
     */
    Data80211PacketType GetDataPacketType();

    /**
     * Gets management frame type.
     * @return management frame type.
     */
    Management80211PacketType GetManagementPacketType();

    /**
     * Gets packet saved in this class.
     * @return string_view with data.
     */
    std::string_view GetPacket();

    /**
     * Sets the source MAC addresses to blacklist.
     */
    void SetMACBlackList(std::vector<uint64_t> aBlackList);

    /**
     * Sets the source MAC addresses to whitelist.
     */
    void SetMACWhiteList(std::vector<uint64_t> aWhiteList);

    /**
     * Sets the SSID to filter on.
     */
    void SetSSIDFilter(std::string_view aSSID);

    /**
     * Preload data about this packet into this class.
     * @param aData - Packet to dissect.
     */
    void Update(std::string_view aPacket);

private:
    // Save last data in this class
    std::string mLastReceivedData{};

    std::vector<uint64_t> aBlackList;
    std::vector<uint64_t> aWhiteList;

    Main80211PacketType       mMainPacketType{Main80211PacketType::None};
    Control80211PacketType    mControlPacketType{Control80211PacketType::None};
    Data80211PacketType       mDataPacketType{Data80211PacketType::None};
    Management80211PacketType mManagementPacketType{Management80211PacketType::None};

    std::shared_ptr<RadioTapReader> mPhysicalDeviceHeaderReader;
};
