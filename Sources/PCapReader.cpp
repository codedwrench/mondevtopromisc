#include "../Includes/PCapReader.h"
#include <string>
#include <iostream>
#include <iomanip>

bool PCapReader::Open(const std::string& aName)
{
    bool lReturn = true;
    char lErrorBuffer[PCAP_ERRBUF_SIZE];
    mHandler = pcap_open_offline(aName.c_str(), lErrorBuffer);
    if (mHandler == nullptr) {
        lReturn = false;
        std::cout << "pcap_open_offline failed, " << lErrorBuffer << std::endl;
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

    if(pcap_next_ex(mHandler, &mHeader, &mData) >= 0) {
        // Show the packet number
        std::cout << "Packet # " << std::dec << ++mPacketCount << std::endl;

        // Show the size in bytes of the packet
        std::cout << "Packet size: " << mHeader->len << " bytes" << std::endl;

        // Show a warning if the length captured is different
        if (mHeader->len != mHeader->caplen)
            std::cout << "Warning! Capture size different than packet size:" << mHeader->len << " bytes" << std::endl;

        // Show Epoch Time
        std::cout << "Epoch Time: " << mHeader->ts.tv_sec << ":" << mHeader->ts.tv_usec << std::endl;

        // Loop through the packet and print it as hexidecimal representations of octets
        for (unsigned int lCount = 0; lCount < mHeader->caplen; lCount++) {
            // Start printing on the next line after every 16 octets
            if (lCount % 16 == 0) {
                std::cout << std::endl;
            }

            std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(mData[lCount]) << " ";
        }

        // Add two lines between packets
        std::cout << std::endl << std::endl;
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