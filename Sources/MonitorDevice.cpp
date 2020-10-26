#include "../Includes/MonitorDevice.h"

#include "../Includes/NetConversionFunctions.h"

/* Copyright (c) 2020 [Rick de Bondt] - MonitorDevice.cpp */

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include <boost/thread.hpp>

#include "../Includes/Logger.h"

using namespace std::chrono;

bool MonitorDevice::Open(std::string_view aName, std::vector<std::string>& aSSIDFilter)
{
    bool lReturn{true};

    mPacketHandler.SetSSIDFilterList(aSSIDFilter);

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

void MonitorDevice::BlackList(uint64_t aMAC)
{
    mPacketHandler.AddToMACBlackList(aMAC);
}

void MonitorDevice::Close()
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

    mHandler            = nullptr;
    mData               = nullptr;
    mHeader             = nullptr;
    mReceiverThread     = nullptr;
    mAcknowledgePackets = false;
}

bool MonitorDevice::ReadCallback(const unsigned char* aData, const pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    mPacketHandler.Update(lData);

    if (mAcknowledgePackets && mPacketHandler.IsAckable()) {
        std::string lAcknowledgementFrame = ConstructAcknowledgementFrame(
            mPacketHandler.GetDestinationMAC(), mPacketHandler.GetControlPacketParameters());

        Send(lAcknowledgementFrame);
    }

    // If this packet is convertible to something XLink can understand, send
    if (mPacketHandler.ShouldSend()) {
        if (Logger::GetInstance().GetLogLevel() == Logger::Level::TRACE) {
            ShowPacketStatistics(aData, aHeader);
        }

        std::string lPacket{mPacketHandler.ConvertPacket()};
        mConnector->Send(lPacket);
    }

    mData = aData;
    mHeader = aHeader;

    return lReturn;
}

void MonitorDevice::ShowPacketStatistics(const unsigned char* aData, const pcap_pkthdr* aHeader)
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

const unsigned char* MonitorDevice::GetData()
{
    return mData;
}

const RadioTapReader::PhysicalDeviceParameters& MonitorDevice::GetDataPacketParameters()
{
    return mPacketHandler.GetDataPacketParameters();
}

const pcap_pkthdr* MonitorDevice::GetHeader()
{
    return mHeader;
}

uint64_t MonitorDevice::GetLockedBSSID()
{
    return mPacketHandler.GetLockedBSSID();
}

std::string MonitorDevice::DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader)
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

bool MonitorDevice::Send(std::string_view aData)
{
    bool lReturn{false};
    if (mHandler != nullptr) {
        std::string lData{};

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

void MonitorDevice::SetConnector(std::shared_ptr<IConnector> aDevice)
{
    mConnector = aDevice;
}

bool MonitorDevice::StartReceiverThread()
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
                        auto* lThis = reinterpret_cast<MonitorDevice*>(aThis);
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

void MonitorDevice::SetSourceMACToFilter(uint64_t aMac)
{
    if (aMac != 0) {
        mPacketHandler.AddToMACWhiteList(aMac);
    }
}

void MonitorDevice::SetAcknowledgePackets(bool aAcknowledge)
{
    mAcknowledgePackets = aAcknowledge;
}
