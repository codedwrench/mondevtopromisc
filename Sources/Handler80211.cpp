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

Main80211PacketType Handler80211::GetMainPacketType()
{
    Main80211PacketType lReturn{Main80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lMainType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 2U)};
        if ((lMainType & ~0b00U) == 0b00U) {
            lReturn = Main80211PacketType::Management;
        } else if ((lMainType & 0b01U) == 0b01U) {
            lReturn = Main80211PacketType::Control;
        } else if ((lMainType & 0b10U) == 0b10U) {
            lReturn = Main80211PacketType::Data;
        }
        // Ignore extensions
    }

    return lReturn;
}

Control80211PacketType Handler80211::GetControlPacketType()
{
    Control80211PacketType lReturn{Control80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lControlType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};

        if ((lControlType & 0b0010U) == 0b0010U) {
            lReturn = Control80211PacketType::Trigger;
        } else if ((lControlType & 0b0011U) == 0b0011U) {
            lReturn = Control80211PacketType::TACK;
        } else if ((lControlType & 0b1000U) == 0b1000U) {
            lReturn = Control80211PacketType::BlockAckRequest;
        } else if ((lControlType & 0b1001U) == 0b1001U) {
            lReturn = Control80211PacketType::BlockAck;
        } else if ((lControlType & 0b1010U) == 0b1010U) {
            lReturn = Control80211PacketType::PSPoll;
        } else if ((lControlType & 0b1011U) == 0b1011U) {
            lReturn = Control80211PacketType::RTS;
        } else if ((lControlType & 0b1100U) == 0b1100U) {
            lReturn = Control80211PacketType::CTS;
        } else if ((lControlType & 0b1101U) == 0b1101U) {
            lReturn = Control80211PacketType::ACK;
        } else {
            Logger::GetInstance().Log("Could not determine control packet type: " + std::to_string(lControlType),
                                      Logger::Level::DEBUG);
        }
    }

    return lReturn;
}

Data80211PacketType Handler80211::GetDataPacketType()
{
    Data80211PacketType lReturn{Control80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lDataType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};

        if ((lDataType & ~0b0000U) == 0b0000U) {
            lReturn = Data80211PacketType::Data;
        } else if ((lDataType & 0b0001U) == 0b0001U) {
            lReturn = Data80211PacketType::DataCFACK;
        } else if ((lDataType & 0b0010U) == 0b0010U) {
            lReturn = Data80211PacketType::DataCFPoll;
        } else if ((lDataType & 0b0011U) == 0b0011U) {
            lReturn = Data80211PacketType::DataCFACKCFPoll;
        } else if ((lDataType & 0b0100U) == 0b0100U) {
            lReturn = Data80211PacketType::Null;
        } else if ((lDataType & 0b0101U) == 0b0101U) {
            lReturn = Data80211PacketType::CFACK;
        } else if ((lDataType & 0b0110U) == 0b0110U) {
            lReturn = Data80211PacketType::CFPoll;
        } else if ((lDataType & 0b0111U) == 0b0111U) {
            lReturn = Data80211PacketType::CFACKCFPoll;
        } else if ((lDataType & 0b1000U) == 0b1000U) {
            lReturn = Data80211PacketType::QoSData;
        } else if ((lDataType & 0b1001U) == 0b1001U) {
            lReturn = Data80211PacketType::QoSDataCFACK;
        } else if ((lDataType & 0b1010U) == 0b1010U) {
            lReturn = Data80211PacketType::QoSDataCFPoll;
        } else if ((lDataType & 0b1011U) == 0b1011U) {
            lReturn = Data80211PacketType::QoSDataCFACKCFPoll;
        } else if ((lDataType & 0b1100U) == 0b1100U) {
            lReturn = Data80211PacketType::QoSNull;
        } else if ((lDataType & 0b1110U) == 0b1110U) {
            lReturn = Data80211PacketType::QoSCFPoll;
        } else if ((lDataType & 0b1111U) == 0b1111U) {
            lReturn = Data80211PacketType::QoSCFACKCFPoll;
        } else {
            Logger::GetInstance().Log("Could not determine data packet type: " + std::to_string(lDataType),
                                      Logger::Level::DEBUG);
        }
    }

    return lReturn;
}

Management80211PacketType Handler80211::GetManagementPacketType()
{
    Management80211PacketType lReturn{Control80211PacketType::None};

    // Makes more sense if you read this: https://en.wikipedia.org/wiki/802.11_Frame_Types#Frame_Control
    if (mPhysicalDeviceHeaderReader != nullptr) {
        uint8_t lManagementType{static_cast<uint8_t>(
            GetRawData<uint8_t>(mLastReceivedData, mPhysicalDeviceHeaderReader->GetLength()) >> 4U)};

        if ((lManagementType & ~0b0000U) == 0b0000U) {
            lReturn = Management80211PacketType::AssociationRequest;
        } else if ((lManagementType & 0b0001U) == 0b0001U) {
            lReturn = Management80211PacketType::AssociationResponse;
        } else if ((lManagementType & 0b0010U) == 0b0010U) {
            lReturn = Management80211PacketType::ReassociationRequest;
        } else if ((lManagementType & 0b0011U) == 0b0011U) {
            lReturn = Management80211PacketType::ReassociationResponse;
        } else if ((lManagementType & 0b0100U) == 0b0100U) {
            lReturn = Management80211PacketType::ProbeRequest;
        } else if ((lManagementType & 0b0101U) == 0b0101U) {
            lReturn = Management80211PacketType::ProbeResponse;
        } else if ((lManagementType & 0b1000U) == 0b1000U) {
            lReturn = Management80211PacketType::Beacon;
        } else if ((lManagementType & 0b1010U) == 0b1010U) {
            lReturn = Management80211PacketType::Disassociation;
        } else if ((lManagementType & 0b1011U) == 0b1011U) {
            lReturn = Management80211PacketType::Authentication;
        } else if ((lManagementType & 0b1100U) == 0b1100U) {
            lReturn = Management80211PacketType::Deauthentication;
        } else if ((lManagementType & 0b1101U) == 0b1101U) {
            lReturn = Management80211PacketType::Action;
        } else if ((lManagementType & 0b1110U) == 0b1110U) {
            lReturn = Management80211PacketType::ActionNoAck;
        } else {
            Logger::GetInstance().Log("Could not determine management packet type: " + std::to_string(lManagementType),
                                      Logger::Level::DEBUG);
        }
        // Ignore the rest
    }

    return lReturn;
}

std::string_view Handler80211::GetPacket()
{
    return mLastReceivedData;
}

void Handler80211::Update(std::string_view aData)
{
    // Save data in object and fill RadioTap parameters.
    mLastReceivedData = aData;
    if (mPhysicalDeviceHeaderReader != nullptr) {
        mPhysicalDeviceHeaderReader->FillRadioTapParameters(aData);
    }

    mMainPacketType = GetMainPacketType();

    switch (mMainPacketType) {
        case Main80211PacketType::Control:
            mControlPacketType = GetControlPacketType();
            break;
        case Main80211PacketType::Data:
            mDataPacketType = GetDataPacketType();
            break;
        case Main80211PacketType::Management:
            mManagementPacketType = GetManagementPacketType();
            break;
        default:
            Logger::GetInstance().Log("Could not determine main packet type", Logger::Level::DEBUG);
    }
}
