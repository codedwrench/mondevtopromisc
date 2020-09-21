#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingHeaders.h
 *
 * This file contains networking headers needed to convert packets between the different formats.
 *
 **/

#include <cstdint>

// Defined in include/linux/ieee80211.h
/**
 * This is the IEEE80211 header.
 **/
struct ieee80211_hdr
{
    uint16_t /*__le16*/ frame_control; /**< see https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control. */
    uint16_t /*__le16*/ duration_id;   /**< Duration or ID, depending on frametype, see 802.11-2016 standard. */
    uint8_t             addr1[6];      /**< Address 1 based on frame_control. */
    uint8_t             addr2[6];      /**< Address 2 based on frame_control. */
    uint8_t             addr3[6];      /**< Address 3 based on frame_control. */
    uint16_t /*__le16*/ seq_ctrl;      /**< Sequence and fragmentation number. */
    // uint8_t addr4[6]; /**< Usually not used. Depends on framecontrol. */
} __attribute__((packed));

/**
 * The Radiotap header needed to construct a WiFi packet.
 **/
struct RadioTapHeader
{
    uint8_t  radiotap_version;     /**< Version of the RadioTap standard. */
    uint8_t  radiotap_version_pad; /**< Padding. */
    uint16_t bytes_in_header;      /**< Amount of bytes in the header. */

    // Bitmap with options:
    // TSFT
    // Flags
    // Rate
    // Channel
    // FHSS
    // dBm Antenna Signal
    // dBm Antenna Noise
    // Lock Quality
    // TX Attenuation
    // dB TX Attenuation
    // dBm TX Power
    // Antenna
    // dB Antenna Signal
    // dB Antenna Noise
    // RX flags
    // TX flags
    // RTS retries
    // Data retries
    // MCS
    // AMPDU_Status
    // VHT
    // Timestamp
    uint32_t present_flags; /**< Which parts of the RadioTap header are present. See source code for more info. */
} __attribute__((packed));

namespace Net_80211_Constants
{
    constexpr uint8_t  cDestinationAddressIndex{4};
    constexpr uint8_t  cDestinationAddressLength{6};
    constexpr uint8_t  cSourceAddressIndex{10};
    constexpr uint8_t  cSourceAddressLength{6};
    constexpr uint8_t  cBSSIDIndex{16};
    constexpr uint8_t  cBSSIDLength{6};
    constexpr uint8_t  cTypeIndex{30};
    constexpr uint8_t  cTypeLength{2};
    constexpr uint8_t  cDataIndex{32};
    constexpr uint8_t  cRadioTapLengthIndex{2};
    constexpr uint8_t  cDataType{0x08};
    constexpr uint8_t  cDataQOSType{0x80};
    constexpr uint8_t  cDataQOSLength{2};
    constexpr uint8_t  cHeaderLength{cDestinationAddressLength + cSourceAddressLength + cTypeLength};
    constexpr uint16_t cWlanFCTypeData{0x0008};
    constexpr uint8_t  cFCSLength{4};

    // 0xAA 0xAA 0x03 is for enabling SNAP
    constexpr uint64_t cSnapLLC{0x000000000003aaaa};
}  // namespace Net_80211_Constants

namespace Net_8023_Constants
{
    constexpr uint8_t cDestinationAddressIndex{0};
    constexpr uint8_t cDestinationAddressLength{6};
    constexpr uint8_t cSourceAddressIndex{6};
    constexpr uint8_t cSourceAddressLength{6};
    constexpr uint8_t cEtherTypeIndex{12};
    constexpr uint8_t cEtherTypeLength{2};
    constexpr uint8_t cDataIndex{14};
    constexpr uint8_t cHeaderLength{cDestinationAddressLength + cSourceAddressLength + cEtherTypeLength};
}  // namespace Net_8023_Constants

namespace RadioTap_Constants
{
    // If when receiving the radiotap header length is higher than this, assume the packet is broken.
    constexpr uint16_t cMaxRadioTapLength{64};

    // Note padding for these options is embedded in the different variables, so if adding a variable that's requiring
    // an alignment, make the variable a step bigger. Also do not forget to add the variable to cRadioTapSize and to
    // InsertRadioTapHeader in PacketConverter.cpp

    // Enable: Flags, Rate, Channel and TX Flags
    constexpr uint32_t cSendPresentFlags{0x0000800e};

    // Short preamble
    constexpr uint8_t cFlags{0x02};

    // Channel 1 (2412hz) only for now
    // TODO: Deduce from incoming traffic
    constexpr uint16_t cChannel{0x096c};       // 0x096c = 2412hz
    constexpr uint16_t cChannelFlags{0x00a0};  // 2.4Ghz, Turbo on

    // Using 11mbps for Vita and PSP traffic
    // TODO: Deduce from incoming traffic.
    constexpr uint8_t cRateFlags{0x16};

    // No Ack
    constexpr uint32_t cTXFlags{0x00000008};

    // RadioTapHeader length with all the optional options:
    constexpr uint16_t cRadioTapSize = sizeof(RadioTapHeader) + sizeof(RadioTap_Constants::cFlags) +
                                       sizeof(RadioTap_Constants::cChannel) +
                                       sizeof(RadioTap_Constants::cChannelFlags) +
                                       sizeof(RadioTap_Constants::cRateFlags) + sizeof(RadioTap_Constants::cTXFlags);


}  // namespace RadioTap_Constants
