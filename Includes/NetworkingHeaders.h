#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingHeaders.h
 *
 * This file contains networking headers needed to convert packets between the different formats.
 *
 **/

#include <cstdint>

#ifdef __GNUC__
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

/**
 * This is the Acknowledgement header.
 **/
PACK(struct AcknowledgementHeader {
    uint16_t /*__le16*/ frame_control;   /**< see https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control. */
    uint16_t /*__le16*/ duration_id;     /**< Duration or ID, depending on frametype, see 802.11-2016 standard. */
    uint8_t             recv_address[6]; /**< Receiver address. */
    // uint8_t addr4[6]; /**< Usually not used. Depends on framecontrol. */
});

// Defined in include/linux/ieee80211.h
/**
 * This is the IEEE80211 header.
 **/
PACK(struct ieee80211_hdr {
    uint16_t /*__le16*/ frame_control; /**< see https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control. */
    uint16_t /*__le16*/ duration_id;   /**< Duration or ID, depending on frametype, see 802.11-2016 standard. */
    uint8_t             addr1[6];      /**< Address 1 based on frame_control. */
    uint8_t             addr2[6];      /**< Address 2 based on frame_control. */
    uint8_t             addr3[6];      /**< Address 3 based on frame_control. */
    uint16_t /*__le16*/ seq_ctrl;      /**< Sequence and fragmentation number. */
    // uint8_t addr4[6]; /**< Usually not used. Depends on framecontrol. */
});

/**
 * Enum containing the main 80211 packet types: Control, Data, Management
 */
enum class Main80211PacketType
{
    Control = 0,
    Data,
    Management,
    None
};

/**
 * Enum containing the 80211 Control types.
 */
enum class Control80211PacketType
{
    ACK = 0,
    BlockAck,
    BlockAckRequest,
    None
};

/**
 * Enum containing the 80211 Control types.
 */
enum class Data80211PacketType
{
    Data,
    Null,
    QoSData,
    QoSNull,
    None
};

/**
 * Enum containing the 80211 Management types.
 */
enum class Management80211PacketType
{
    Action = 0,
    ActionNoAck,
    Authentication,
    AssociationRequest,
    AssociationResponse,
    Beacon,
    Deauthentication,
    Disassociation,
    ReassociationRequest,
    ReassociationResponse,
    ProbeRequest,
    ProbeResponse,
    None
};

/**
 * Enum containing PhysicalDeviceHeaderType, like RadioTap.
 * @note Only RadioTap supported as of now.
 */
enum class PhysicalDeviceHeaderType
{
    RadioTap = 0, /**< RadioTap type. */
    Prism,        /**< Old PRISM type. */
    AVS,          /**< Even older AVS type. */
    None          /**< No Physical device header at all. */
};

/**
 * The Radiotap header needed to construct a WiFi packet.
 **/
PACK(struct RadioTapHeader {
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
});

namespace Net_8023_Constants
{
    static constexpr uint8_t cDestinationAddressIndex{0};
    static constexpr uint8_t cDestinationAddressLength{6};
    static constexpr uint8_t cSourceAddressIndex{6};
    static constexpr uint8_t cSourceAddressLength{6};
    static constexpr uint8_t cEtherTypeIndex{12};
    static constexpr uint8_t cEtherTypeLength{2};
    static constexpr uint8_t cDataIndex{14};
    static constexpr uint8_t cHeaderLength{cDestinationAddressLength + cSourceAddressLength + cEtherTypeLength};
}  // namespace Net_8023_Constants

namespace Net_80211_Constants
{
    // Frame types
    static constexpr uint16_t cBeaconType{0x80};
    static constexpr uint8_t  cDataType{0x08};
    static constexpr uint8_t  cDataQOSType{0x88};
    static constexpr uint8_t  cDataRetryFlag{0x08};
    static constexpr uint8_t  cDataNullFuncType{0x48};
    static constexpr uint8_t  cAcknowledgementType{0xd4};

    // IEEE 802.11 Data Flags
    static constexpr uint8_t cTypeIndex{0};
    static constexpr uint8_t cTypeLength{2};
    static constexpr uint8_t cDurationIndex{2};
    static constexpr uint8_t cDurationLength{2};
    static constexpr uint8_t cDestinationAddressIndex{4};
    static constexpr uint8_t cDestinationAddressLength{6};
    static constexpr uint8_t cSourceAddressIndex{10};
    static constexpr uint8_t cSourceAddressLength{6};
    static constexpr uint8_t cBSSIDIndex{16};
    static constexpr uint8_t cBSSIDLength{6};
    static constexpr uint8_t cFragmentNumberIndex{22};
    static constexpr uint8_t cFragmentNumberLength{2};
    static constexpr uint8_t c80211DataHeaderLength{cTypeLength + cDurationLength + cDestinationAddressLength +
                                                    cSourceAddressLength + cBSSIDLength + cFragmentNumberLength};

    // IEEE 802.11 Wireless Management
    static constexpr uint8_t cFixedParameterTypeSSIDIndex{36};
    static constexpr uint8_t cFixedParameterTypeSupportedRates{0x1};
    static constexpr uint8_t cFixedParameterTypeDSParameterSet{0x3};
    static constexpr uint8_t cFixedParameterTypeExtendedRates{0x32};

    // LLC
    // 0xAA 0xAA 0x03 is for enabling SNAP
    static constexpr uint64_t cSnapLLC{0x000000000003aaaa};

    static constexpr uint8_t cEtherTypeIndex{30};
    static constexpr uint8_t cEtherTypeLength{2};
    static constexpr uint8_t cLLCLength{8};

    // IEEE 802.11 Data
    static constexpr uint8_t cDataIndex{32};
    static constexpr uint8_t cDataQOSLength{2};

    static constexpr uint16_t cWlanFCTypeData{0x0008};

    static constexpr uint8_t cFCSLength{4};
    static constexpr uint8_t cDataHeaderLength{c80211DataHeaderLength + cLLCLength};
}  // namespace Net_80211_Constants

namespace RadioTap_Constants
{
    static constexpr uint8_t cLengthIndex{2};
    static constexpr uint8_t cPresentFlagsIndex{4};

    // If when receiving the radiotap header length is higher than this, assume the packet is broken.
    static constexpr uint16_t cMaxLength{64};

    // Note padding for these options is embedded in the different variables, so if adding a variable that's requiring
    // an alignment, make the variable a step bigger. Also do not forget to add the variable to cRadioTapSize and to
    // InsertRadioTapHeader in NetConversionFunctions.h

    // Enable: Flags, Rate, Channel and TX Flags
    static constexpr uint32_t cSendPresentFlags{0x0000800e};

    // Short preamble
    static constexpr uint8_t cFlags{0x02};
    static constexpr uint8_t cFCSAvailableFlag{0x10};

    // Channel 1 (2412hz)
    static constexpr uint16_t cChannel{0x096c};       // 0x096c = 2412hz
    static constexpr uint16_t cChannelFlags{0x00c0};  // 2.4Ghz, Turbo on

    // Using 11mbps for Vita and PSP traffic
    static constexpr uint8_t cRateFlags{0x30};

    // No Ack
    static constexpr uint16_t cTXFlags{0x0008};

    // RadioTapHeader length with all the optional options:
    static constexpr uint16_t cRadioTapSize =
        sizeof(RadioTapHeader) + sizeof(RadioTap_Constants::cFlags) + sizeof(RadioTap_Constants::cChannel) +
        sizeof(RadioTap_Constants::cChannelFlags) + sizeof(RadioTap_Constants::cRateFlags) +
        sizeof(RadioTap_Constants::cTXFlags);


}  // namespace RadioTap_Constants
