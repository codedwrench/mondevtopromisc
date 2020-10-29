#include "../Includes/PCapReader.h"

/* Copyright (c) 2020 [Rick de Bondt] - PCapReader.cpp */

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

using namespace std::chrono;

PCapReader::PCapReader(bool aMonitorCapture) : mMonitorCapture(aMonitorCapture)
{
    if (mMonitorCapture) {
        mPacketHandler = std::make_shared<Handler80211>();
    } else {
        mPacketHandler = std::make_shared<Handler8023>();
    }
}

void PCapReader::Close()
{
    if (mReplayThread->joinable()) {
        mReplayThread->join();
    }

    if (mHandler != nullptr) {
        pcap_close(mHandler);
    }

    mHandler            = nullptr;
    mData               = nullptr;
    mHeader             = nullptr;
    mReplayThread       = nullptr;
    mAcknowledgePackets = false;
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

bool PCapReader::ReadCallback(const unsigned char* aData, pcap_pkthdr* aHeader)
{
    bool lReturn{false};

    // Load all needed information into the handler
    std::string lData{DataToString(aData, aHeader)};
    mPacketHandler->Update(lData);

    if (mMonitorCapture) {
        // If we have a monitor device we want the 80211 handler.
        auto lHandler{std::dynamic_pointer_cast<Handler80211>(mPacketHandler)};

        if (lHandler != nullptr) {
            if (!lHandler->IsDropped()) {
                ShowPacketStatistics(aHeader);
                Logger::GetInstance().Log("Received: " + PrettyHexString(lData), Logger::Level::TRACE);
            }

            if (mAcknowledgePackets && lHandler->IsAckable()) {
                std::string lAcknowledgementFrame = ConstructAcknowledgementFrame(
                    mPacketHandler->GetSourceMAC(), lHandler->GetControlPacketParameters());

                Logger::GetInstance().Log("Sent ACK", Logger::Level::TRACE);
                Send(lAcknowledgementFrame);
            }

            // If this packet is convertible to something XLink can understand, send
            if (lHandler->ShouldSend()) {
                std::string lPacket{lHandler->ConvertPacket()};
                mConnector->Send(lPacket);
            }
        }
    }

    mData   = aData;
    mHeader = aHeader;
    mPacketCount++;

    return lReturn;
}

bool PCapReader::ReadNextData()
{
    bool lReturn = true;

    if (pcap_next_ex(mHandler, &mHeader, &mData) < 0) {
        Logger::GetInstance().Log("Reading offline capture failed: " + std::string(pcap_geterr(mHandler)), Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}

bool PCapReader::Send(std::string_view aData)
{
    bool lReturn{false};
    if (mHandler != nullptr) {
        if (!aData.empty()) {
            Logger::GetInstance().Log(std::string("Would have sent: ") + PrettyHexString(aData), Logger::Level::TRACE);
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

void PCapReader::ShowPacketStatistics(pcap_pkthdr* aHeader) const
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
                if (ReadNextData()) {
                    microseconds lTimeStamp{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec};

                    ReadCallback(mData, mHeader);

                    while (ReadNextData()) {
                        // Get time offset.
                        microseconds lSleepFor{mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec - lTimeStamp.count()};
                        lTimeStamp = microseconds(mHeader->ts.tv_sec * 1000000 + mHeader->ts.tv_usec);

                        // Wait for next send.
                        std::this_thread::sleep_for(lSleepFor);

                        // Wait for next send.
                        ReadCallback(mData, mHeader);
                    }
                }
            });
        } else {
            Logger::GetInstance().Log("Can't start receiving without a handler!", Logger::Level::ERROR);
            lReturn = false;
        }
    }
    return lReturn;
}

void PCapReader::SetSourceMACToFilter(uint64_t aMac)
{
    if (aMac != 0) {
        mPacketHandler->AddToMACWhiteList(aMac);
    }
}

void PCapReader::SetAcknowledgePackets(bool aAcknowledge)
{
    mAcknowledgePackets = aAcknowledge;
}
