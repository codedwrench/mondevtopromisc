#include "../Includes/WirelessMonitorDevice.h"

/* Copyright (c) 2020 [Rick de Bondt] - WirelessMonitorDevice.cpp */

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include <boost/thread.hpp>

#include "../Includes/Logger.h"

using namespace std::chrono;

bool WirelessMonitorDevice::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter, uint16_t aFrequency)
{
    bool lReturn{true};
    mSSIDFilter                = std::move(aSSIDFilter);
    mWifiInformation.Frequency = aFrequency;
    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};

    mHandler = pcap_create(aName.data(), lErrorBuffer.data());
    pcap_set_snaplen(mHandler, cSnapshotLength);
    pcap_set_timeout(mHandler, cTimeout);
    pcap_set_immediate_mode(mHandler, 1);

    int lStatus{pcap_activate(mHandler)};

    if (lStatus == 0) {
        mConnected = true;
    } else {
        lReturn = false;
        Logger::GetInstance().Log("pcap_activate failed, " + std::string(pcap_statustostr(lStatus)),
                                  Logger::Level::ERROR);
    }
    return lReturn;
}

void WirelessMonitorDevice::Close()
{
    mConnected = false;

    if (mHandler != nullptr) {
        pcap_breakloop(mHandler);
    }

    if (mReceiverThread->joinable()) {
        mReceiverThread->join();
    }

    if (mHandler != nullptr) {
        pcap_close(mHandler);
    }

    mHandler               = nullptr;
    mData                  = nullptr;
    mHeader                = nullptr;
    mReceiverThread        = nullptr;
    mWifiInformation.SSID  = "";
    mWifiInformation.BSSID = 0;
    mSourceMACToFilter     = 0;
    mAcknowledgePackets    = false;
}

bool WirelessMonitorDevice::ReadNextData()
{
    bool lReturn{false};
    int  lSuccess{pcap_next_ex(mHandler, const_cast<pcap_pkthdr**>(&mHeader), &mData)};

    if (lSuccess == 1) {
        lReturn = ReadCallback(mData, mHeader);
    } else if (lSuccess == 0) {
        Logger::GetInstance().Log("Packet Timeout", Logger::Level::DEBUG);
    } else if (lSuccess == -1) {
        Logger::GetInstance().Log("Error occurred while reading packet: " + std::string(pcap_geterr(mHandler)),
                                  Logger::Level::DEBUG);
    } else {
        Logger::GetInstance().Log("Unknown error occurred while reading packet", Logger::Level::DEBUG);
    }

    return lReturn;
}

bool WirelessMonitorDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    std::string lData = DataToString(aData, aHeader);

    // Load information about this packet into the packet converter
    mPacketConverter.Update(lData);

    if (mPacketConverter.Is80211Beacon(lData)) {
        // Try to match SSID to filter list
        std::string lSSID = mPacketConverter.GetBeaconSSID(lData);

        for (auto& lFilter : mSSIDFilter) {
            if (lSSID.find(lFilter) != std::string::npos) {
                if (lSSID != mWifiInformation.SSID) {
                    mPacketConverter.FillWiFiInformation(lData, mWifiInformation);
                    Logger::GetInstance().Log("SSID switched:" + lSSID, Logger::Level::DEBUG);
                }
            }
        }
    } else if (mPacketConverter.Is80211Data(lData) && mPacketConverter.IsForBSSID(lData, mWifiInformation.BSSID) &&
               (mSourceMACToFilter == 0 || mPacketConverter.IsFromMac(lData, mSourceMACToFilter))) {
        ++mPacketCount;

        // Don't even bother setting up these strings if loglevel is not trace.
        if (Logger::GetInstance().GetLogLevel() == Logger::Level::TRACE) {
            Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::Level::TRACE);

            // Show the size in bytes of the packet
            Logger::GetInstance().Log("Packet size: " + std::to_string(aHeader->len) + " bytes", Logger::Level::TRACE);

            // Show Epoch Time
            Logger::GetInstance().Log(
                "Epoch time: " + std::to_string(aHeader->ts.tv_sec) + ":" + std::to_string(aHeader->ts.tv_usec),
                Logger::Level::TRACE);

            // Show a warning if the length captured is different
            if (aHeader->len != aHeader->caplen) {
                Logger::GetInstance().Log(
                    "Capture size different than packet size:" + std::to_string(aHeader->len) + " bytes",
                    Logger::Level::TRACE);
            }
        }

        // Data is good so save as member
        mData   = aData;
        mHeader = aHeader;


        if (mAcknowledgePackets) {
            // If it's not a broadcast frame, acknowledge the packet.
            if (mPacketConverter.GetDestinationMac(lData) != 0xFFFFFFFFFFFF) {
                uint64_t               lUnconvertedSourceMac{mPacketConverter.GetSourceMac(lData)};
                // Big- to Little endian
                lUnconvertedSourceMac = mPacketConverter.SwapMacEndian(lUnconvertedSourceMac);

                std::array<uint8_t, 6> lSourceMac{};
                memcpy(reinterpret_cast<char*>(lSourceMac.data()), &lUnconvertedSourceMac, sizeof(uint8_t) * 6);

                std::string lAcknowledgementFrame{mPacketConverter.ConstructAcknowledgementFrame(
                    lSourceMac, mWifiInformation.Frequency, mWifiInformation.MaxRate)};
                Send(lAcknowledgementFrame, mWifiInformation, false);
            }
        }

        if (mSendReceivedData && (mSendReceiveDevice != nullptr) && !mPacketConverter.Is80211QOSRetry(lData)) {
            std::string lConvertedData = mPacketConverter.ConvertPacketTo8023(lData);
            if (!lConvertedData.empty()) {
                mSendReceiveDevice->Send(lConvertedData);
            }
        }


        lReturn = true;
    }

    return lReturn;
}

const unsigned char* WirelessMonitorDevice::GetData()
{
    return mData;
}

const pcap_pkthdr* WirelessMonitorDevice::GetHeader()
{
    return mHeader;
}

std::string WirelessMonitorDevice::DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader)
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

std::string WirelessMonitorDevice::LastDataToString()
{
    return DataToString(mData, mHeader);
}

void WirelessMonitorDevice::SetSSID(std::string_view aSSID)
{
    mWifiInformation.SSID = aSSID;
}

bool WirelessMonitorDevice::Send(std::string_view                              aData,
                                 IPCapDevice_Constants::WiFiBeaconInformation& aWiFiInformation,
                                 bool                                          aConvertData)
{
    bool lReturn{false};
    if (mHandler != nullptr) {
        std::string lData{};

        if (aConvertData) {
            lData = mPacketConverter.ConvertPacketTo80211(
                aData, aWiFiInformation.BSSID, aWiFiInformation.Frequency, aWiFiInformation.MaxRate);
        } else {
            lData = aData;
        }

        if (!lData.empty()) {
            Logger::GetInstance().Log("Sent: " + lData, Logger::Level::TRACE);

            if (pcap_sendpacket(mHandler, reinterpret_cast<const unsigned char*>(lData.c_str()), lData.size()) == 0) {
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

bool WirelessMonitorDevice::Send(std::string_view aData, IPCapDevice_Constants::WiFiBeaconInformation& aWiFiInformation)
{
    return Send(aData, mWifiInformation, true);
}

bool WirelessMonitorDevice::Send(std::string_view aData)
{
    return Send(aData, mWifiInformation);
}

void WirelessMonitorDevice::SetSendReceiveDevice(std::shared_ptr<ISendReceiveDevice> aDevice)
{
    mSendReceiveDevice = aDevice;
}

bool WirelessMonitorDevice::StartReceiverThread()
{
    bool lReturn{true};
    if (mHandler != nullptr) {
        // Run
        if (mReceiverThread == nullptr) {
            mReceiverThread = std::make_shared<boost::thread>([&] {
                // If we're receiving data from the receiver thread, send it off as well.
                bool lSendReceivedDataOld = mSendReceivedData;
                mSendReceivedData         = true;

                auto lCallbackFunction =
                    [](unsigned char* aThis, const pcap_pkthdr* aHeader, const unsigned char* aPacket) {
                        auto* lThis = reinterpret_cast<WirelessMonitorDevice*>(aThis);
                        lThis->ReadCallback(aPacket, aHeader);
                    };

                while (mConnected && (mHandler != nullptr)) {
                    // Use pcap_dispatch instead of pcap_next_ex so that as many packets as possible will be processed
                    // in a single cycle.
                    if (pcap_dispatch(mHandler, -1, lCallbackFunction, reinterpret_cast<u_char*>(this)) == -1) {
                        Logger::GetInstance().Log(
                            "Error occurred while reading packet: " + std::string(pcap_geterr(mHandler)),
                            Logger::Level::DEBUG);
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }

                mSendReceivedData = lSendReceivedDataOld;
            });
        }
    } else {
        Logger::GetInstance().Log("Can't start receiving without a handler!", Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}

void WirelessMonitorDevice::SetSourceMACToFilter(uint64_t aMac)
{
    mSourceMACToFilter = aMac;
}

void WirelessMonitorDevice::SetAcknowledgePackets(bool aAcknowledge)
{
    mAcknowledgePackets = aAcknowledge;
}
