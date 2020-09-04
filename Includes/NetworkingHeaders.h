#ifndef NETWORKINGHEADERS_H
#define NETWORKINGHEADERS_H

#include <cstdint>

namespace Net_80211_Constants
{
    constexpr uint8_t cDestinationAddressIndex{4};
    constexpr uint8_t cDestinationAddressLength{6};
    constexpr uint8_t cSourceAddressIndex{10};
    constexpr uint8_t cSourceAddressLength{6};
    constexpr uint8_t cBSSIDIndex{16};
    constexpr uint8_t cBSSIDLength{6};
    constexpr uint8_t cTypeIndex{30};
    constexpr uint8_t cTypeLength{2};
    constexpr uint8_t cDataIndex{32};
    constexpr uint8_t cRadioTapLengthIndex{2};
    constexpr uint8_t cDataType{0x08};
    constexpr uint8_t cHeaderLength{cDestinationAddressLength +
                                    cSourceAddressLength +
                                    cTypeLength};
    constexpr uint16_t cWlanFCTypeData{0x0080};

    // 0xAA 0xAA 0x03 is for enabling SNAP
    constexpr uint64_t cSnapLLC{0xaaaa030000000000};
}

namespace Net_8023_Constants
{
    constexpr uint8_t cDestinationAddressIndex{0};
    constexpr uint8_t cDestinationAddressLength{6};
    constexpr uint8_t cSourceAddressIndex{6};
    constexpr uint8_t cSourceAddressLength{6};
    constexpr uint8_t cEtherTypeIndex{12};
    constexpr uint8_t cEtherTypeLength{2};
    constexpr uint8_t cEtherDataIndex{14};
    constexpr uint8_t cHeaderLength{cDestinationAddressLength +
                                    cSourceAddressLength +
                                    cEtherTypeLength};
}

namespace RadioTap_Constants
{
    // Enable: TX Flags
    uint32_t cSendPresentFlags{0x00800000};
    // No Ack
    uint16_t cTXFlags{0x0800};
}

// Defined in include/linux/ieee80211.h
struct ieee80211_hdr
{
    uint16_t /*__le16*/ frame_control;
    uint16_t /*__le16*/ duration_id;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t /*__le16*/ seq_ctrl;
    //uint8_t addr4[6];
} __attribute__ ((packed));

struct RadioTapHeader
{
    uint8_t radiotap_version;
    uint8_t radiotap_version_pad;
    uint16_t bytes_in_header;

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
    uint32_t present_flags;
} __attribute__ ((packed));

#endif //NETWORKINGHEADERS_H
