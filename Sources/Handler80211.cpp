#include "../Includes/Handler80211.h"

/* Copyright (c) 2020 [Rick de Bondt] - Handler80211.cpp */

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

Handler80211::Handler80211(PhysicalDeviceHeaderType aType)
{
    if (aType == PhysicalDeviceHeaderType::RadioTap) {
        mPhysicalDeviceHeaderReader = std::make_shared<RadioTapReader>();
    }
}

std::string_view Handler80211::GetPacket()
{
    return mLastReceivedData;
}

bool Handler80211::IsMACAllowed(uint64_t aMAC)
{
    bool lReturn{false};

    if (mWhiteList.empty()) {
        if (std::find(mBlackList.begin(), mBlackList.end(), aMAC) == mBlackList.end()) {
            lReturn = true;
        }
    } else {
        if (std::find(mWhiteList.begin(), mWhiteList.end(), aMAC) != mWhiteList.end()) {
            lReturn = true;
        }
    }

    return lReturn;
}

bool Handler80211::IsSSIDAllowed(std::string_view aSSID)
{
    bool lReturn{false};

    if (mSSIDList.empty() || std::find(mSSIDList.begin(), mSSIDList.end(), aSSID) != mSSIDList.end()) {
        lReturn = true;
    }

    return lReturn;
}

void Handler80211::Update(std::string_view aData)
{
    // Save data in object and fill RadioTap parameters.
    mLastReceivedData = aData;

    if (mPhysicalDeviceHeaderReader != nullptr) {
        mPhysicalDeviceHeaderReader->FillRadioTapParameters(aData);
    }

    UpdateSourceMac();

    if (IsMACAllowed(mSourceMac)) {
        UpdateMainPacketType();

        switch (mMainPacketType) {
            case Main80211PacketType::Control:
                // We don't really do anything with control frames yet, so determining the type is a waste of cycles
                // UpdateControlPacketType();
                break;
            case Main80211PacketType::Data:
                UpdateDataPacketType();
                break;
            case Main80211PacketType::Management:
                UpdateManagementPacketType();

                if (mManagementPacketType == Management80211PacketType::Beacon) {
                    mParameter80211Reader.Update(mLastReceivedData);
                    if (IsSSIDAllowed(mParameter80211Reader.GetSSID())) {
                        uint64_t lBSSID = mBSSID;
                        UpdateBSSID();
                        if (lBSSID != mBSSID) {
                            Logger::GetInstance().Log(std::string("SSID switched:") +
                                                          mParameter80211Reader.GetSSID().data() +
                                                          " , BSSID: " + std::to_string(mBSSID),
                                                      Logger::Level::DEBUG);
                        }
                    }
                }
                break;
            default:
                Logger::GetInstance().Log("Could not determine main packet type", Logger::Level::DEBUG);
        }
    }
}

void Handler80211::UpdateBSSID()
{
    uint64_t lBSSID{0};
    if (mPhysicalDeviceHeaderReader != nullptr) {
        lBSSID = GetRawData<uint64_t>(mLastReceivedData,
                                      mPhysicalDeviceHeaderReader->GetLength() + Net_80211_Constants::cBSSIDIndex);
        lBSSID &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48U) - 1);  // it's actually a uint48.

        // Big- to Little endian
        mBSSID = SwapMacEndian(lBSSID);
    }
}

void Handler80211::UpdateMainPacketType()
{
    Main80211PacketType lResult{Main80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lMainType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 2U)};
        if ((lMainType & ~0b00U) == 0b00U) {
            lResult = Main80211PacketType::Management;
        } else if ((lMainType & 0b01U) == 0b01U) {
            lResult = Main80211PacketType::Control;
        } else if ((lMainType & 0b10U) == 0b10U) {
            lResult = Main80211PacketType::Data;
        }
        // Ignore extensions
    }

    mMainPacketType = lResult;
}

void Handler80211::UpdateControlPacketType()
{
    Control80211PacketType lResult{Control80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lControlType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};

        if ((lControlType & 0b0010U) == 0b0010U) {
            lResult = Control80211PacketType::Trigger;
        } else if ((lControlType & 0b0011U) == 0b0011U) {
            lResult = Control80211PacketType::TACK;
        } else if ((lControlType & 0b1000U) == 0b1000U) {
            lResult = Control80211PacketType::BlockAckRequest;
        } else if ((lControlType & 0b1001U) == 0b1001U) {
            lResult = Control80211PacketType::BlockAck;
        } else if ((lControlType & 0b1010U) == 0b1010U) {
            lResult = Control80211PacketType::PSPoll;
        } else if ((lControlType & 0b1011U) == 0b1011U) {
            lResult = Control80211PacketType::RTS;
        } else if ((lControlType & 0b1100U) == 0b1100U) {
            lResult = Control80211PacketType::CTS;
        } else if ((lControlType & 0b1101U) == 0b1101U) {
            lResult = Control80211PacketType::ACK;
        } else {
            Logger::GetInstance().Log("Could not determine control packet type: " + std::to_string(lControlType),
                                      Logger::Level::DEBUG);
        }
    }

    mControlPacketType = lResult;
}

void Handler80211::UpdateDataPacketType()
{
    Data80211PacketType lResult{Control80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lDataType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};

        if ((lDataType & ~0b0000U) == 0b0000U) {
            lResult = Data80211PacketType::Data;
        } else if ((lDataType & 0b0001U) == 0b0001U) {
            lResult = Data80211PacketType::DataCFACK;
        } else if ((lDataType & 0b0010U) == 0b0010U) {
            lResult = Data80211PacketType::DataCFPoll;
        } else if ((lDataType & 0b0011U) == 0b0011U) {
            lResult = Data80211PacketType::DataCFACKCFPoll;
        } else if ((lDataType & 0b0100U) == 0b0100U) {
            lResult = Data80211PacketType::Null;
        } else if ((lDataType & 0b0101U) == 0b0101U) {
            lResult = Data80211PacketType::CFACK;
        } else if ((lDataType & 0b0110U) == 0b0110U) {
            lResult = Data80211PacketType::CFPoll;
        } else if ((lDataType & 0b0111U) == 0b0111U) {
            lResult = Data80211PacketType::CFACKCFPoll;
        } else if ((lDataType & 0b1000U) == 0b1000U) {
            lResult = Data80211PacketType::QoSData;
        } else if ((lDataType & 0b1001U) == 0b1001U) {
            lResult = Data80211PacketType::QoSDataCFACK;
        } else if ((lDataType & 0b1010U) == 0b1010U) {
            lResult = Data80211PacketType::QoSDataCFPoll;
        } else if ((lDataType & 0b1011U) == 0b1011U) {
            lResult = Data80211PacketType::QoSDataCFACKCFPoll;
        } else if ((lDataType & 0b1100U) == 0b1100U) {
            lResult = Data80211PacketType::QoSNull;
        } else if ((lDataType & 0b1110U) == 0b1110U) {
            lResult = Data80211PacketType::QoSCFPoll;
        } else if ((lDataType & 0b1111U) == 0b1111U) {
            lResult = Data80211PacketType::QoSCFACKCFPoll;
        } else {
            Logger::GetInstance().Log("Could not determine data packet type: " + std::to_string(lDataType),
                                      Logger::Level::DEBUG);
        }
    }

    mDataPacketType = lResult;
}

void Handler80211::UpdateManagementPacketType()
{
    Management80211PacketType lResult{Control80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lManagementType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};

        if ((lManagementType & ~0b0000U) == 0b0000U) {
            lResult = Management80211PacketType::AssociationRequest;
        } else if ((lManagementType & 0b0001U) == 0b0001U) {
            lResult = Management80211PacketType::AssociationResponse;
        } else if ((lManagementType & 0b0010U) == 0b0010U) {
            lResult = Management80211PacketType::ReassociationRequest;
        } else if ((lManagementType & 0b0011U) == 0b0011U) {
            lResult = Management80211PacketType::ReassociationResponse;
        } else if ((lManagementType & 0b0100U) == 0b0100U) {
            lResult = Management80211PacketType::ProbeRequest;
        } else if ((lManagementType & 0b0101U) == 0b0101U) {
            lResult = Management80211PacketType::ProbeResponse;
        } else if ((lManagementType & 0b1000U) == 0b1000U) {
            lResult = Management80211PacketType::Beacon;
        } else if ((lManagementType & 0b1010U) == 0b1010U) {
            lResult = Management80211PacketType::Disassociation;
        } else if ((lManagementType & 0b1011U) == 0b1011U) {
            lResult = Management80211PacketType::Authentication;
        } else if ((lManagementType & 0b1100U) == 0b1100U) {
            lResult = Management80211PacketType::Deauthentication;
        } else if ((lManagementType & 0b1101U) == 0b1101U) {
            lResult = Management80211PacketType::Action;
        } else if ((lManagementType & 0b1110U) == 0b1110U) {
            lResult = Management80211PacketType::ActionNoAck;
        } else {
            Logger::GetInstance().Log("Could not determine management packet type: " + std::to_string(lManagementType),
                                      Logger::Level::DEBUG);
        }
        // Ignore the rest
    }

    mManagementPacketType = lResult;
}

void Handler80211::UpdateSourceMac()
{
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint64_t lSourceMac{GetRawData<uint64_t>(
            mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength() + Net_80211_Constants::cSourceAddressIndex)};
        lSourceMac &= static_cast<uint64_t>(static_cast<uint64_t>(1LLU << 48U) - 1);  // it's actually a uint48.

        // Big- to Little endian
        lSourceMac = bswap_64(lSourceMac);
        mSourceMac = lSourceMac >> 16U;
    }
}