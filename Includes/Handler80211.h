#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Handler80211.h
 *
 * This file reads packets from a monitor format and converts to a promiscuous format.
 *
 **/

#include <memory>
#include <string>
#include <vector>

#include "NetworkingHeaders.h"
#include "Parameter80211Reader.h"
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
     * Add the source MAC address to blacklist.
     * @param aMac - MAC address to blacklist.
     */
    void AddToMACBlackList(uint64_t aMAC);

    /**
     * Add the source MAC address to whitelist. Whitelist takes prevalence over the blacklist.
     * @param aMac - MAC address to whitelist.
     */
    void AddToMACWhiteList(uint64_t aMAC);

    /**
     * Clears blacklist.
     */
    void ClearMACBlackList();

    /**
     * Clears whitelist.
     */
    void ClearMACWhiteList();

    /**
     * This function converts a monitor mode packet to a promiscuous mode packet, stripping the radiotap and
     * 802.11 header and adding an 802.3 header. Only converts data packets!
     * @param aData - The packet data to convert.
     * @return converted packet data, empty string if failed.
     */
    std::string ConvertPacket();

    /**
     * Gets parameters for a control packet type, for example used for constructing acknowledgement frames.
     * @return a reference to PhysicalDeviceParameters object with the needed parameters.
     */
    const RadioTapReader::PhysicalDeviceParameters& GetControlPacketParameters();

    /**
     * Gets parameters for a data packet type, for example used for conversion to an 80211 packet.
     * @return a reference to PhysicalDeviceParameters object with the needed parameters.
     */
    const RadioTapReader::PhysicalDeviceParameters& GetDataPacketParameters();

    /**
     * Gets destination MAC address of packet.
     * @return the destination MAC address.
     */
    uint64_t GetDestinationMAC() const;

    /**
     * Gets locked onto BSSID.
     * @return the locked onto BSSID.
     */
    uint64_t GetLockedBSSID() const;

    /**
     * Gets packet saved in this class.
     * @return string_view with data.
     */
    std::string_view GetPacket();

    /**
     * Checks if a packet is ackable.
     * @return true is packet is ackable.
     */
    bool IsAckable() const;

    /**
     * Checks if this BSSID is locked onto.
     * @param aBSSID - BSSID to check
     * @return true if BSSID is locked onto.
     */
    [[nodiscard]] bool IsBSSIDAllowed(uint64_t aBSSID) const;

    /**
     * Checks if MAC is in receiver blacklist.
     * @param aMAC - MAC to check
     * @return true if MAC is blacklisted.
     */
    [[nodiscard]] bool IsMACBlackListed(uint64_t aMAC) const;

    /**
     * Checks if last received packet is convertible to 802.3 and should be sent.
     * @return true if should be sent.
     */
    bool ShouldSend();

    /**
     * Checks if this MAC is not blacklisted / whitelisted.
     * @param aMAC - MAC to check
     * @return true if MAC address is allowed.
     */
    bool IsMACAllowed(uint64_t aMAC);

    /**
     * Checks if this SSID is whitelisted.
     * @param aSSID - SSID to check
     * @return true if SSID is whitelisted.
     */
    bool IsSSIDAllowed(std::string_view aSSID);

    /**
     *  Saves PhysicalDeviceParameters struct to reference.
     *  @param aParameters - reference to struct containing those parameters.
     */
    void SavePhysicalDeviceParameters(RadioTapReader::PhysicalDeviceParameters& aParameters);

    /**
     * Sets the source MAC addresses blacklist.
     */
    void SetMACBlackList(std::vector<uint64_t>& aBlackList);

    /**
     * Sets the source MAC addresses whitelist.
     */
    void SetMACWhiteList(std::vector<uint64_t>& aWhiteList);

    /**
     * Sets the SSID to filter on.
     */
    void SetSSIDFilterList(std::vector<std::string>& aSSIDList);

    /**
     * Preload data about this packet into this class.
     * @param aData - Packet to dissect.
     */
    void Update(std::string_view aPacket);

private:
    void UpdateBSSID();
    void UpdateControlPacketType();
    void UpdateDataPacketType();
    void UpdateMainPacketType();
    void UpdateManagementPacketType();
    void UpdateAckable();
    void UpdateQOSRetry();
    void UpdateDestinationMac();
    void UpdateSourceMac();

    // Save last data in this class
    std::string mLastReceivedData{};

    std::vector<uint64_t>    mBlackList{};
    std::vector<std::string> mSSIDList{};
    std::vector<uint64_t>    mWhiteList{};

    Main80211PacketType       mMainPacketType{Main80211PacketType::None};
    Control80211PacketType    mControlPacketType{Control80211PacketType::None};
    Data80211PacketType       mDataPacketType{Data80211PacketType::None};
    Management80211PacketType mManagementPacketType{Management80211PacketType::None};

    bool     mAckable{false};
    uint64_t mBSSID{0};
    uint64_t mDestinationMac{0};
    uint64_t mLockedBSSID{0};
    bool     mQOSRetry{false};
    bool     mShouldSend{false};
    uint64_t mSourceMac{0};

    std::shared_ptr<Parameter80211Reader> mParameter80211Reader{nullptr};
    std::shared_ptr<RadioTapReader> mPhysicalDeviceHeaderReader{nullptr};

    RadioTapReader::PhysicalDeviceParameters mPhysicalDeviceParametersControl{};
    RadioTapReader::PhysicalDeviceParameters mPhysicalDeviceParametersData{};
};
