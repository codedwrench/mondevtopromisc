#include "../Includes/WirelessPSPPluginDevice.h"

#include "../Includes/NetConversionFunctions.h"

/* Copyright (c) 2021 [Rick de Bondt] - WirelessPSPPluginDevice.cpp */

#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include "../Includes/Logger.h"

using namespace std::chrono;

// Keeping the SSID filter in because of future autoconnect
bool WirelessPSPPluginDevice::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter)
{
    bool lReturn{true};

    mWifiInterface = std::make_shared<WifiInterface>(aName);
    mSSIDFilter    = aSSIDFilter;
    ConnectToAdhoc();

    std::array<char, PCAP_ERRBUF_SIZE> lErrorBuffer{};

    mHandler = pcap_create(aName.data(), lErrorBuffer.data());
    pcap_set_snaplen(mHandler, cSnapshotLength);
    pcap_set_timeout(mHandler, cPCAPTimeoutMs);
    // TODO: Test without immediate mode, see if it helps
    // pcap_set_immediate_mode(mHandler, 1);

    int lStatus{pcap_activate(mHandler)};

    if (lStatus == 0) {
        mConnected         = true;
        mAdapterMACAddress = mWifiInterface->GetAdapterMACAddress();
        // Do not try to negiotiate with localhost
        BlackList(mAdapterMACAddress);
    } else {
        lReturn = false;
        Logger::GetInstance().Log("pcap_activate failed, " + std::string(pcap_statustostr(lStatus)),
                                  Logger::Level::ERROR);
    }

    // Reset the timer
    mReadWatchdog = std::chrono::system_clock::now();

    return lReturn;
}

void WirelessPSPPluginDevice::Close()
{
    mConnected = false;

    if (mHandler != nullptr) {
        pcap_breakloop(mHandler);
    }

    if (mReceiverThread != nullptr) {
        while (!mReceiverThread->joinable()) { 
            // Wait 
        };
        mReceiverThread->join();
    }

    if (mWifiTimeoutThread != nullptr) {
        while (!mWifiTimeoutThread->joinable()) { 
            // Wait
        };
        mWifiTimeoutThread->join();
    }

    if (mHandler != nullptr) {
        pcap_close(mHandler);
    }

    mHandler        = nullptr;
    mData           = nullptr;
    mHeader         = nullptr;
    mReceiverThread = nullptr;
    mWifiInterface  = nullptr;
}

bool WirelessPSPPluginDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    uint64_t    lSourceMac{
        (GetRawData<uint64_t>(lData, Net_8023_Constants::cSourceAddressIndex) & Net_Constants::cBroadcastMac)};

    // Reset the timer so it will not time out
    mReadWatchdog = std::chrono::system_clock::now();

    if (!IsMACBlackListed(lSourceMac)) {
        if (((GetRawData<uint64_t>(lData, Net_8023_Constants::cDestinationAddressIndex) &
              Net_Constants::cBroadcastMac) == Net_Constants::cBroadcastMac) &&
            (GetRawData<uint16_t>(lData, Net_8023_Constants::cEtherTypeIndex) == Net_Constants::cPSPEtherType)) {
            // Obtain required MACs
            uint64_t lAdapterMAC = mAdapterMACAddress;
            auto     lPSPMAC     = GetRawData<uint64_t>(lData, Net_8023_Constants::cSourceAddressIndex);

            // Tell the PSP what Mac address to use
            std::string lPacket{};
            // Reserve size
            lPacket.resize(Net_8023_Constants::cHeaderLength + Net_8023_Constants::cSourceAddressLength);

            // Now put it into a packet
            int lIndex{0};
            memcpy(lPacket.data(), &lPSPMAC, Net_8023_Constants::cDestinationAddressLength);
            lIndex += Net_8023_Constants::cDestinationAddressLength;

            memcpy(lPacket.data() + lIndex, &lAdapterMAC, Net_8023_Constants::cSourceAddressLength);
            lIndex += Net_8023_Constants::cSourceAddressLength;

            memcpy(lPacket.data() + lIndex, &Net_Constants::cPSPEtherType, Net_8023_Constants::cEtherTypeLength);
            lIndex += Net_8023_Constants::cEtherTypeLength;

            memcpy(lPacket.data() + lIndex, &lAdapterMAC, Net_8023_Constants::cDestinationAddressLength);

            Send(lPacket, false);
        } else {
            // With the plugin the destination mac is kept at the end of the packet
            std::string lActualDestinationMac{
                lData.substr(lData.size() - Net_8023_Constants::cDestinationAddressLength)};
            lData.replace(Net_8023_Constants::cDestinationAddressIndex,
                          Net_8023_Constants::cDestinationAddressLength,
                          lActualDestinationMac);
            lData.resize(lData.size() - Net_8023_Constants::cDestinationAddressLength);
            mConnector->Send(lData);

            mData   = aData;
            mHeader = aHeader;
            mPacketCount++;
        }
    }

    return lReturn;
}

void WirelessPSPPluginDevice::BlackList(uint64_t aMAC)
{
    if (!IsMACBlackListed(aMAC)) {
        Logger::GetInstance().Log("Added: " + IntToMac(aMAC) + " to blacklist.", Logger::Level::TRACE);
        mBlackList.push_back(aMAC);
    }
}

void WirelessPSPPluginDevice::ClearMACBlackList()
{
    mBlackList.clear();
}

bool WirelessPSPPluginDevice::IsMACBlackListed(uint64_t aMAC) const
{
    bool lReturn{false};

    if (std::find(mBlackList.begin(), mBlackList.end(), aMAC) != mBlackList.end()) {
        lReturn = true;
    }

    return lReturn;
}
void WirelessPSPPluginDevice::ShowPacketStatistics(const pcap_pkthdr* aHeader) const
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

const unsigned char* WirelessPSPPluginDevice::GetData()
{
    return mData;
}

const pcap_pkthdr* WirelessPSPPluginDevice::GetHeader()
{
    return mHeader;
}

std::string WirelessPSPPluginDevice::DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader)
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

bool WirelessPSPPluginDevice::Send(std::string_view aData)
{
    return Send(aData, true);
}

bool WirelessPSPPluginDevice::Send(std::string_view aData, bool aModifyData)
{
    bool lReturn{false};
    if (mHandler != nullptr) {
        if (!aData.empty()) {
            std::string lData{aData.data(), aData.size()};

            if (aModifyData) {
                uint64_t lAdapterMAC = mAdapterMACAddress;

                std::string lActualSourceMac{
                    lData.substr(Net_8023_Constants::cSourceAddressIndex, Net_8023_Constants::cSourceAddressLength)};

                memcpy(lData.data() + Net_8023_Constants::cSourceAddressIndex,
                       &lAdapterMAC,
                       Net_8023_Constants::cSourceAddressLength);

                lData.append(lActualSourceMac);
            }

            Logger::GetInstance().Log(std::string("Sent: ") + PrettyHexString(lData), Logger::Level::TRACE);

            if (pcap_sendpacket(mHandler, reinterpret_cast<const unsigned char*>(lData.data()), lData.size()) == 0) {
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

bool WirelessPSPPluginDevice::ConnectToAdhoc()
{
    bool lReturn{};
    // Send leave IBSS command just in case
    mWifiInterface->LeaveIBSS();
    std::vector<IWifiInterface::WifiInformation>& lNetworks = mWifiInterface->GetAdhocNetworks();
    for (const auto& lNetwork : lNetworks) {
        for (const auto& lFilter : mSSIDFilter) {
            if (lNetwork.ssid.find(lFilter) != std::string::npos && lNetwork.isadhoc && !lNetwork.isconnected) {
                lReturn = mWifiInterface->Connect(lNetwork);
            }
        }
    }
    return lReturn;
}

void WirelessPSPPluginDevice::SetConnector(std::shared_ptr<IConnector> aDevice)
{
    mConnector = aDevice;
}

bool WirelessPSPPluginDevice::StartReceiverThread()
{
    bool lReturn{true};

    if (mHandler != nullptr) {
        // Run
        if (mReceiverThread == nullptr) {
            if (mWifiTimeoutThread == nullptr) {
                mWifiTimeoutThread = std::make_shared<std::thread>([&] {
                    while (mConnected) {
                        if (std::chrono::system_clock::now() >
                            (mReadWatchdog + WirelessPSPPluginDevice_Constants::cReadWatchdogTimeout)) {
                            Logger::GetInstance().Log("Switching networks due to timeout!", Logger::Level::DEBUG);
                            // Read timed out try to connect to another network.
                            ConnectToAdhoc();
                            mReadWatchdog = std::chrono::system_clock::now();
                        }
                        std::this_thread::sleep_for(1s);
                    }
                });
            }

            mReceiverThread = std::make_shared<std::thread>([&] {
                // If we're receiving data from the receiver thread, send it off as well.
                bool lSendReceivedDataOld = mSendReceivedData;
                mSendReceivedData         = true;
                mReadWatchdog             = std::chrono::system_clock::now();

                auto lCallbackFunction =
                    [](unsigned char* aThis, const pcap_pkthdr* aHeader, const unsigned char* aPacket) {
                        auto* lThis = reinterpret_cast<WirelessPSPPluginDevice*>(aThis);
                        lThis->ReadCallback(aPacket, aHeader);
                    };

                while (mConnected && (mHandler != nullptr)) {
                    // Use pcap_dispatch instead of pcap_next_ex so that as many packets as possible will be processed
                    // in a single cycle.
                    if (pcap_dispatch(mHandler, 0, lCallbackFunction, reinterpret_cast<u_char*>(this)) == -1) {
                        Logger::GetInstance().Log(
                            "Error occurred while reading packet: " + std::string(pcap_geterr(mHandler)),
                            Logger::Level::DEBUG);
                    }
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
