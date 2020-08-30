#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

#include "../Includes/Logger.h"
#include "../Includes/PacketConverter.h"
#include "../Includes/PCapReader.h"

using namespace std::chrono;

bool PCapReader::Open(const std::string& aName)
{
    bool lReturn = true;
    char lErrorBuffer[PCAP_ERRBUF_SIZE];
    mHandler = pcap_open_offline(aName.c_str(), lErrorBuffer);
    if (mHandler == nullptr) {
        lReturn = false;
        Logger::GetInstance().Log("pcap_open_offline failed, " + std::string(lErrorBuffer), Logger::ERROR);
    }
    return lReturn;
}

void PCapReader::Close()
{
    pcap_close(mHandler);
    mData = nullptr;
    mHeader = nullptr;
}

bool PCapReader::ReadNextPacket()
{
    bool lReturn = true;

    if (pcap_next_ex(mHandler, &mHeader, &mData) >= 0) {
        ++mPacketCount;
        Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::TRACE);

        // Show the size in bytes of the packet
        Logger::GetInstance().Log("Packet size: " + std::to_string(mHeader->len) + " bytes", Logger::TRACE);


        // Show a warning if the length captured is different
        if (mHeader->len != mHeader->caplen) {
            Logger::GetInstance().Log("Capture size different than packet size:" + std::to_string(mHeader->len) +
                                      " bytes", Logger::WARNING);
        }

        // Show Epoch Time
        Logger::GetInstance().Log("Epoch time: " + std::to_string(mHeader->ts.tv_sec) + ":" +
                                  std::to_string(mHeader->ts.tv_usec), Logger::TRACE);
    } else {
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


// TODO: Needs to be some interface with send function for all interfaces
std::pair<bool, unsigned int> PCapReader::ReplayPackets(XLinkKaiConnection& aConnection, bool aMonitorCapture)
{
    bool lFailedPacket{false};
    bool lUnimportantPacket{false};
    unsigned int lPacketsSent = 0;

    // Read the first packet
    if (ReadNextPacket()) {
        microseconds lTimeStamp{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec};

        std::string lData = DataToString();

        if (aMonitorCapture) {
            // TODO: constant!
            // If Data Frame
            if (mData[0] == 0x08) {
                lData = PacketConverter::ConvertPacketToPromiscuous(lData);
                if (lData.empty()) {
                    lFailedPacket = true;
                }
            } else {
                lUnimportantPacket = true;
            }
        }

        lData.insert(0, XLinkKai_Constants::cEthernetDataString);

        if (lUnimportantPacket || ReplayPacket(aConnection, lData)) {
            if (!lUnimportantPacket) {
                lPacketsSent++;
            }

            bool lSuccessfulReplay{true};

            while (ReadNextPacket() && lSuccessfulReplay) {
                lUnimportantPacket = false;

                // Get time offset.
                microseconds lSleepFor{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec - lTimeStamp.count()};
                lTimeStamp = microseconds(mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec);

                // Wait for next send.
                std::this_thread::sleep_for(lSleepFor);

                // Convert to promiscuous packet and proceed as normal.
                if (aMonitorCapture) {
                    // TODO: constant!
                    // If Data Frame
                    if (mData[0] == 0x08) {
                        lData = PacketConverter::ConvertPacketToPromiscuous(DataToString());
                        if (lData.empty()) {
                            lFailedPacket = true;
                        }
                    } else {
                        lUnimportantPacket = true;
                    }
                }

                if (!lFailedPacket && !lUnimportantPacket) {
                    lData.insert(0, XLinkKai_Constants::cEthernetDataString);
                    lSuccessfulReplay = ReplayPacket(aConnection, lData);
                    if (lSuccessfulReplay) {
                        lPacketsSent++;
                    } else {
                        lFailedPacket = true;
                    }
                }
            }
        }
    }

    return std::pair{!lFailedPacket, lPacketsSent};
}

bool PCapReader::ReplayPacket(XLinkKaiConnection& aConnection, std::string_view aData)
{
    bool lReturn{false};

    if (aConnection.Send(aData)) {
        lReturn = true;
    }

    return lReturn;
}
