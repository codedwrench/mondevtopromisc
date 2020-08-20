#include <iomanip>
#include <iostream>
#include <string>

#include "../Includes/Logger.h"
#include "../Includes/PCapReader.h"

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

bool PCapReader::ReadNextPacket() {
    bool lReturn = true;

    if (pcap_next_ex(mHandler, &mHeader, &mData) >= 0) {
        ++mPacketCount;
        Logger::GetInstance().Log("Packet # " + std::to_string(mPacketCount), Logger::DEBUG);

        // Show the size in bytes of the packet
        Logger::GetInstance().Log("Packet size: " + std::to_string(mHeader->len) + " bytes", Logger::DEBUG);


        // Show a warning if the length captured is different
        if (mHeader->len != mHeader->caplen) {
            Logger::GetInstance().Log("Capture size different than packet size:" + std::to_string(mHeader->len) +
                                              " bytes", Logger::WARNING);
        }

        // Show Epoch Time
        Logger::GetInstance().Log("Epoch time: " + std::to_string(mHeader->ts.tv_sec) + ":" +
                                          std::to_string(mHeader->ts.tv_usec), Logger::DEBUG);
    } else {
        lReturn = false;
    }

    return lReturn;
}

const unsigned char *PCapReader::GetData() {
    return mData;
}

const pcap_pkthdr *PCapReader::GetHeader() {
    return mHeader;
}

std::string PCapReader::DataToFormattedString() {
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