#include "../Includes/PCapReader.h"

/* Copyright (c) 2020 [Rick de Bondt] - PCapReader.cpp */

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

PCapReader::PCapReader(bool aMonitorCapture) : mMonitorCapture(aMonitorCapture)
{
    if (mMonitorCapture) {
        mPacketHandler = std::make_shared<Handler80211>();
    } else {
        mPacketHandler = std::make_shared<Handler8023>();
    }
}

std::string PCapReader::DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    // Convert from char* to string
    std::string lData{};

    if ((aData != nullptr) && (aHeader != nullptr)) {
        lData.resize(aHeader->caplen);
        for (unsigned int lCount = 0; lCount < aHeader->caplen; lCount++) {
            lData.at(lCount) = aData[lCount];
        }
    }

    return lData;
}

const unsigned char* PCapReader::GetData()
{
    return mData;
}

const pcap_pkthdr* PCapReader::GetHeader()
{
    return mHeader;
}

bool PCapReader::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter)
{
    bool                               lReturn{true};
    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};
    mHandler = pcap_open_offline(aName.data(), lErrorBuffer.data());
    if (mHandler == nullptr) {
        lReturn = false;
        Logger::GetInstance().Log("pcap_open_offline failed, " + std::string(lErrorBuffer.data()),
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

bool PCapReader::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    mPacketHandler->Update(lData);

    if (mMonitorCapture) {
        // If we have a monitor device we want the 80211 handler.
        auto lHandler{std::dynamic_pointer_cast<Handler80211>(mPacketHandler)};

        if (lHandler != nullptr) {
            if (mAcknowledgePackets && lHandler->IsAckable()) {
                std::string lAcknowledgementFrame =
                    ConstructAcknowledgementFrame(lHandler->GetSourceMAC(), lHandler->GetControlPacketParameters());

                Send(lAcknowledgementFrame);
            }

            // If this packet is convertible to something XLink can understand, send
            if (lHandler->ShouldSend()) {
                if (Logger::GetInstance().GetLogLevel() == Logger::Level::TRACE) {
                    ShowPacketStatistics(aHeader);
                }

                std::string lPacket{lHandler->ConvertPacket()};
                mConnector->Send(lPacket);
            }
        }
    }

    mData   = aData;
    mHeader = aHeader;

    return lReturn;
}

bool PCapReader::Send(std::string_view aData)
{
    bool lReturn{false};
    if (mHandler != nullptr) {
        if (!aData.empty()) {
            Logger::GetInstance().Log(std::string("Sent: ") + aData.data(), Logger::Level::TRACE);

            if (pcap_sendpacket(mHandler, reinterpret_cast<const unsigned char*>(aData.data()), aData.size()) == 0) {
                lReturn = true;
            } else {
                Logger::GetInstance().Log("pcap_sendpacket failed, " + std::string(pcap_geterr(mHandler)),
                                          Logger::Level::ERROR);
            }
        }
    } else {
        Logger::GetInstance().Log("Cannot send packets on a device that has not been opened yet!",
                                  Logger::Level::ERROR);
    }

    return lReturn;
}

void PCapReader::SetConnector(std::shared_ptr<IConnector> aDevice)
{
    mConnector = aDevice;
}


void PCapReader::ShowPacketStatistics(const pcap_pkthdr* aHeader) const
{
    Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::Level::TRACE);

    // Show the size in bytes of the packet
    Logger::GetInstance().Log("Packet size: " + std::to_string(aHeader->len) + " bytes", Logger::Level::TRACE);

    // Show Epoch Time
    Logger::GetInstance().Log(
        "Epoch time: " + std::to_string(aHeader->ts.tv_sec) + ":" + std::to_string(aHeader->ts.tv_usec),
        Logger::Level::TRACE);

    // Show a warning if the length captured is different
    if (aHeader->len != aHeader->caplen) {
        Logger::GetInstance().Log("Capture size different than packet size:" + std::to_string(aHeader->len) + " bytes",
                                  Logger::Level::TRACE);
    }
}

bool PCapReader::StartReceiverThread()
{
    bool lReturn{true};
    if (mHandler != nullptr) {
        // Run
        if (mReplayThread == nullptr) {
            mReplayThread = std::make_shared<boost::thread>([&] {
                auto lCallbackFunction =
                    [](unsigned char* aThis, const pcap_pkthdr* aHeader, const unsigned char* aPacket) {
                        auto* lThis = reinterpret_cast<PCapReader*>(aThis);
                        lThis->ReadCallback(aPacket, aHeader);
                    };

                while (mConnected && (mHandler != nullptr)) {
                    // Use pcap_dispatch instead of pcap_next_ex so that as many packets as possible will be processed
                    // in a single cycle.
                    // TODO: I wonder what this does with a file.
                    if (pcap_dispatch(mHandler, -1, lCallbackFunction, reinterpret_cast<u_char*>(this)) == -1) {
                        Logger::GetInstance().Log(
                            "Error occurred while reading packet: " + std::string(pcap_geterr(mHandler)),
                            Logger::Level::DEBUG);
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            });
        }
    } else {
        Logger::GetInstance().Log("Can't start receiving without a handler!", Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}

//#include <chrono>
//#include <functional>
//#include <iomanip>
//#include <iostream>
//#include <string>
//
//#include "../Includes/Logger.h"
// using namespace std::chrono;
//
// bool PCapReader::Open(std::string_view aName, uint16_t aFrequency)
//{
//    bool                               lReturn{true};
//    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};
//    mHandler = pcap_open_offline(aName.data(), lErrorBuffer.data());
//    if (mHandler == nullptr) {
//        lReturn = false;
//        Logger::GetInstance().Log("pcap_open_offline failed, " + std::string(lErrorBuffer.data()),
//                                  Logger::Level::ERROR);
//    }
//    mWifiInformation.Frequency = aFrequency;
//
//    return lReturn;
//}
//
// bool PCapReader::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter, uint16_t aFrequency)
//{
//    bool lReturn = Open(aName, aFrequency);
//    mSSIDFilter  = aSSIDFilter;
//
//    return lReturn;
//}
//
// void PCapReader::Close()
//{
//    pcap_close(mHandler);
//    mData   = nullptr;
//    mHeader = nullptr;
//}
//
// bool PCapReader::ReadNextData()
//{
//    bool lReturn = true;
//
//    if (mHandler != nullptr) {
//        if (pcap_next_ex(mHandler, &mHeader, &mData) >= 0) {
//            ++mPacketCount;
//            Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::Level::TRACE);
//
//            // Show the size in bytes of the packet
//            Logger::GetInstance().Log("Packet size: " + std::to_string(mHeader->len) + " bytes",
//            Logger::Level::TRACE);
//
//
//            // Show a warning if the length captured is different
//            if (mHeader->len != mHeader->caplen) {
//                Logger::GetInstance().Log(
//                    "Capture size different than packet size:" + std::to_string(mHeader->len) + " bytes",
//                    Logger::Level::WARNING);
//            }
//
//            // Show Epoch Time
//            Logger::GetInstance().Log(
//                "Epoch time: " + std::to_string(mHeader->ts.tv_sec) + ":" + std::to_string(mHeader->ts.tv_usec),
//                Logger::Level::TRACE);
//        } else {
//            lReturn = false;
//        }
//    } else {
//        Logger::GetInstance().Log("Handler not initialized before call", Logger::Level::DEBUG);
//        lReturn = false;
//    }
//
//    return lReturn;
//}
//
// const unsigned char* PCapReader::GetData()
//{
//    return mData;
//}
//
// const pcap_pkthdr* PCapReader::GetHeader()
//{
//    return mHeader;
//}
//
// std::string PCapReader::DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader)
//{
//    // Convert from char* to string
//    std::string lData{};
//
//    if ((aData != nullptr) && (aHeader != nullptr)) {
//        lData.resize(aHeader->caplen);
//        for (unsigned int lCount = 0; lCount < aHeader->caplen; lCount++) {
//            lData.at(lCount) = aData[lCount];
//        }
//    }
//
//    return lData;
//}
//
// std::string PCapReader::LastDataToString()
//{
//    return DataToString(mData, mHeader);
//}
//
// std::pair<bool, bool> PCapReader::ConstructAndReplayPacket(const unsigned char* aData,
//                                                           const pcap_pkthdr*   aHeader,
//                                                           PacketConverter      aPacketConverter,
//                                                           bool                 aMonitorCapture)
//{
//    bool lUsefulPacket{true};
//    bool lSuccesfulPacket{true};
//
//    std::string lData{DataToString(aData, aHeader)};
//
//    if (aMonitorCapture) {
//        lUsefulPacket = false;
//
//        if (aPacketConverter.Is80211Beacon(lData)) {
//            // Try to match SSID to filter list
//            std::string lSSID = aPacketConverter.GetBeaconSSID(lData);
//
//            for (auto& lFilter : mSSIDFilter) {
//                if (lSSID.find(lFilter) != std::string::npos) {
//                    if (lSSID != mWifiInformation.SSID) {
//                        aPacketConverter.Fill80211WiFiInformation(lData, mWifiInformation);
//                        Logger::GetInstance().Log("SSID switched:" + lSSID, Logger::Level::DEBUG);
//                    }
//                }
//            }
//        } else if (aPacketConverter.Is80211Data(lData) && aPacketConverter.IsForBSSID(lData, mWifiInformation.BSSID))
//        {
//            ++mPacketCount;
//            lUsefulPacket = true;
//        }
//    }
//
//    if ((mSendReceiveDevice != nullptr) && lUsefulPacket) {
//        if (aMonitorCapture) {
//            lData = aPacketConverter.ConvertPacketTo8023(lData);
//        }
//        if (!lData.empty()) {
//            lUsefulPacket = true;
//            if (!mSendReceiveDevice->Send(lData)) {
//                lSuccesfulPacket = false;
//            }
//        }
//    }
//
//    return {lSuccesfulPacket, lUsefulPacket};
//}
//
// std::pair<bool, unsigned int> PCapReader::ReplayPackets(bool aMonitorCapture, bool aHasRadioTap)
//{
//    bool         lSuccesfulPacket{false};
//    unsigned int lPacketsSent{0};
//
//    if (mSendReceiveDevice != nullptr) {
//        bool lUsefulPacket{true};
//        // Read the first packet
//        if (ReadNextData()) {
//            PacketConverter lPacketConverter{aHasRadioTap};
//            microseconds    lTimeStamp{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec};
//
//            std::tie(lSuccesfulPacket, lUsefulPacket) =
//                ConstructAndReplayPacket(mData, mHeader, lPacketConverter, aMonitorCapture);
//
//            if (lSuccesfulPacket && lUsefulPacket) {
//                lPacketsSent++;
//            }
//
//            while (ReadNextData()) {
//                // Get time offset.
//                microseconds lSleepFor{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec - lTimeStamp.count()};
//                lTimeStamp = microseconds(mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec);
//
//                // Wait for next send.
//                std::this_thread::sleep_for(lSleepFor);
//
//                std::tie(lSuccesfulPacket, lUsefulPacket) =
//                    ConstructAndReplayPacket(mData, mHeader, lPacketConverter, aMonitorCapture);
//
//                if (lSuccesfulPacket && lUsefulPacket) {
//                    lPacketsSent++;
//                }
//            }
//        }
//    } else {
//        Logger::GetInstance().Log("Cannot replay packets wihout a send/receive device set!", Logger::Level::ERROR);
//    }
//
//    return std::pair{lSuccesfulPacket, lPacketsSent};
//}
//
// bool PCapReader::Send(std::string_view /*aData*/, IPCapDevice_Constants::WiFiBeaconInformation& /*aWiFiInformation*/)
//{
//    return false;
//}
//
// bool PCapReader::Send(std::string_view /*aData*/)
//{
//    // Maybe make it possible to inject a packet into the capture file here, but not sure if that's beneficial.
//    return false;
//}
//
// void PCapReader::SetConnector(std::shared_ptr<IConnector> aDevice)
//{
//    mSendReceiveDevice = aDevice;
//}
//
// const IPCapDevice_Constants::WiFiBeaconInformation& PCapReader::GetWifiInformation()
//{
//    return mWifiInformation;
//}
