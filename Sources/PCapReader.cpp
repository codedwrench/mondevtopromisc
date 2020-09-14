#include "../Includes/PCapReader.h"

/* Copyright (c) 2020 [Rick de Bondt] - PCapReader.cpp */

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

#include "../Includes/Logger.h"
using namespace std::chrono;

bool PCapReader::Open(std::string_view aName)
{
    bool lReturn{true};
    char lErrorBuffer[PCAP_ERRBUF_SIZE];
    mHandler = pcap_open_offline(aName.data(), lErrorBuffer);
    if (mHandler == nullptr) {
        lReturn = false;
        Logger::GetInstance().Log("pcap_open_offline failed, " + std::string(lErrorBuffer), Logger::ERR);
    }
    return lReturn;
}

void PCapReader::Close()
{
    pcap_close(mHandler);
    mData   = nullptr;
    mHeader = nullptr;
}

bool PCapReader::ReadNextData()
{
    bool lReturn = true;

    if (mHandler != nullptr) {
        if (pcap_next_ex(mHandler, &mHeader, &mData) >= 0) {
            ++mPacketCount;
            Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::TRACE);

            // Show the size in bytes of the packet
            Logger::GetInstance().Log("Packet size: " + std::to_string(mHeader->len) + " bytes", Logger::TRACE);


            // Show a warning if the length captured is different
            if (mHeader->len != mHeader->caplen) {
                Logger::GetInstance().Log(
                    "Capture size different than packet size:" + std::to_string(mHeader->len) + " bytes",
                    Logger::WARNING);
            }

            // Show Epoch Time
            Logger::GetInstance().Log(
                "Epoch time: " + std::to_string(mHeader->ts.tv_sec) + ":" + std::to_string(mHeader->ts.tv_usec),
                Logger::TRACE);
        } else {
            lReturn = false;
        }
    } else {
        Logger::GetInstance().Log("Handler not initialized before call", Logger::DEBUG);
        lReturn = false;
    }

    return lReturn;
}

const unsigned char* PCapReader::GetData()
{
    return mData;
}

const pcap_pkthdr* PCapReader::GetHeader()
{
    return mHeader;
}

std::string PCapReader::DataToFormattedString()
{
    std::stringstream lFormattedString;
    // Loop through the packet and print it as hexidecimal representations of octets
    for (unsigned int lCount = 0; lCount < mHeader->caplen; lCount++) {
        // Start printing on the next line after every 16 octets
        if (lCount % 16 == 0) {
            lFormattedString << std::endl;
        }

        lFormattedString << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(mData[lCount])
                         << " ";
    }

    return lFormattedString.str();
}

std::string PCapReader::DataToString()
{
    // Convert from char* to string
    std::string lData{};
    lData.reserve(mHeader->caplen);
    for (unsigned int lCount = 0; lCount < mHeader->caplen; lCount++) {
        lData += mData[lCount];
    }

    return lData;
}

std::pair<bool, bool> PCapReader::ConstructAndReplayPacket(ISendReceiveDevice& aConnection,
                                                           PacketConverter     aPacketConverter,
                                                           bool                aMonitorCapture)
{
    bool        lUsefulPacket{true};
    bool        lSuccesfulPacket{true};
    std::string lData{DataToString()};

    // Convert to promiscuous packet and proceed as normal.
    if (aMonitorCapture) {
        // If Data Frame
        lUsefulPacket = aPacketConverter.Is80211Data(lData);
        if (lUsefulPacket) {
            lData = aPacketConverter.ConvertPacketTo8023(lData);
            if (lData.empty()) {
                lSuccesfulPacket = false;
            }
        }
    }

    if (lSuccesfulPacket && lUsefulPacket) {
        if (!aConnection.Send(lData)) {
            lSuccesfulPacket = false;
        }
    }

    return {lSuccesfulPacket, lUsefulPacket};
}

std::pair<bool, unsigned int> PCapReader::ReplayPackets(bool aMonitorCapture, bool aHasRadioTap)
{
    bool         lSuccesfulPacket{false};
    unsigned int lPacketsSent{0};

    if (mSendReceiveDevice != nullptr) {
        bool lUsefulPacket{true};
        // Read the first packet
        if (ReadNextData()) {
            PacketConverter lPacketConverter{aHasRadioTap};
            microseconds    lTimeStamp{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec};

            std::tie(lSuccesfulPacket, lUsefulPacket) =
                ConstructAndReplayPacket(*mSendReceiveDevice, lPacketConverter, aMonitorCapture);

            if (lSuccesfulPacket && lUsefulPacket) {
                lPacketsSent++;
            }

            while (ReadNextData()) {
                // Get time offset.
                microseconds lSleepFor{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec - lTimeStamp.count()};
                lTimeStamp = microseconds(mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec);

                // Wait for next send.
                std::this_thread::sleep_for(lSleepFor);

                std::tie(lSuccesfulPacket, lUsefulPacket) =
                    ConstructAndReplayPacket(*mSendReceiveDevice, lPacketConverter, aMonitorCapture);

                if (lSuccesfulPacket && lUsefulPacket) {
                    lPacketsSent++;
                }
            }
        }
    } else {
        Logger::GetInstance().Log("Cannot replay packets wihout a send/receive device set!", Logger::ERR);
    }

    return std::pair{lSuccesfulPacket, lPacketsSent};
}

bool PCapReader::Send(std::string_view aData)
{
    // Maybe make it possible to inject a packet into the capture file here, but not sure if that's beneficial.
    return false;
}

void PCapReader::SetSendReceiveDevice(std::shared_ptr<ISendReceiveDevice> aDevice)
{
    mSendReceiveDevice = aDevice;
}
