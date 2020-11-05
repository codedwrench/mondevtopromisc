/* Copyright (c) 2020 [Rick de Bondt] - PacketConverter_Test.cpp
 * This file contains tests for the PacketConverter class.
 **/


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/PCapReader.h"
#include "../Includes/XLinkKaiConnection.h"


using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::WithArg;
class IConnectorMock : public IConnector
{
public:
    MOCK_METHOD(bool, Open, (std::string_view aArgument));
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(std::string, LastDataToString, ());
    MOCK_METHOD(bool, ReadNextData, ());
    MOCK_METHOD(bool, Send, (std::string_view aData));
    MOCK_METHOD(void, SetIncomingConnection, (std::shared_ptr<IPCapDevice> aDevice));
    MOCK_METHOD(bool, StartReceiverThread, ());
};

class IPCapDeviceMock : public IPCapDevice
{
public:
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(bool, Open, (std::string_view aName, std::vector<std::string>& aSSIDFilter));
    MOCK_METHOD(std::string, DataToString, (const unsigned char* aData, const pcap_pkthdr* aHeader));
    MOCK_METHOD(const unsigned char*, GetData, ());
    MOCK_METHOD(const pcap_pkthdr*, GetHeader, ());
    MOCK_METHOD(bool, Send, (std::string_view aData));
    MOCK_METHOD(void, SetConnector, (std::shared_ptr<IConnector> aDevice));
    MOCK_METHOD(bool, StartReceiverThread, ());
};

class PacketHandlingTest : public ::testing::Test
{
protected:
    Handler80211 mHandler80211{PhysicalDeviceHeaderType::RadioTap};
    Handler8023  mHandler8023{};
};

// Tests whether MacToInt can successfully convert a well formed MAC address string.
TEST_F(PacketHandlingTest, MacToInt)
{
    uint64_t aResult{MacToInt("01:23:45:67:AB:CD")};
    ASSERT_EQ(aResult, 0x01234567abcd);
}

TEST_F(PacketHandlingTest, MonitorToPromiscuous)
{
    std::shared_ptr<IConnector> lConnector{std::make_shared<IConnectorMock>()};
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    PCapReader                  lPCapReader{true, false};
    PCapReader                  lPCapExpectedReader{false, false};


    pcap_t*           lHandler        = pcap_open_dead(DLT_EN10MB, 65535);
    const std::string lOutputFileName = "../Tests/Output/MonitorToPromiscuousOutput.pcap";
    pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());

    std::vector<std::string> lSSIDFilter{"T#STNET"};
    lPCapReader.Open("../Tests/Input/MonitorHelloWorld.pcapng", lSSIDFilter);
    lPCapExpectedReader.Open("../Tests/Input/MonitorToPromiscuousOutput_Expected.pcap");

    lPCapReader.SetConnector(lConnector);
    lPCapExpectedReader.SetConnector(lExpectedConnector);

    std::vector<std::string> lSendBuffer{};
    std::vector<timeval>     lTimeStamp{};

    std::vector<std::string> lSendExpectedBuffer{};


    // This piece of magic will grab sent output for me and save its timestamps
    EXPECT_CALL(*std::dynamic_pointer_cast<IConnectorMock>(lConnector), Send(_))
        .WillRepeatedly(DoAll(WithArg<0>([&](std::string_view aMessage) {
                                  lSendBuffer.emplace_back(aMessage);
                                  lTimeStamp.push_back(lPCapReader.GetHeader()->ts);
                              }),
                              Return(true)));


    EXPECT_CALL(*std::dynamic_pointer_cast<IConnectorMock>(lExpectedConnector), Send(_))
        .WillRepeatedly(DoAll(
            WithArg<0>([&](std::string_view aMessage) { lSendExpectedBuffer.emplace_back(std::string(aMessage)); }),
            Return(true)));

    lPCapReader.StartReceiverThread();
    lPCapExpectedReader.StartReceiverThread();

    while (!lPCapReader.IsDoneReceiving() || !lPCapExpectedReader.IsDoneReceiving()) {}

    ASSERT_EQ(lSendBuffer.size(), lSendExpectedBuffer.size());

    int lCount{0};
    for (auto& lMessage : lSendBuffer) {
        pcap_pkthdr lHeader{};
        lHeader.caplen = lMessage.size();
        lHeader.len    = lMessage.size();
        lHeader.ts     = lTimeStamp.at(lCount);

        // Output a file with the results as well so the results can be further inspected
        pcap_dump(reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const u_char*>(lMessage.c_str()));

        ASSERT_EQ(lMessage, lSendExpectedBuffer.at(lCount));
        lCount++;
    }

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
}

TEST_F(PacketHandlingTest, PromiscuousToMonitor)
{
    std::shared_ptr<PCapReader> lConnector{std::make_shared<PCapReader>(true, false)};
    std::shared_ptr<IPCapDeviceMock> lIncomingConnection{std::make_shared<IPCapDeviceMock>()};    
    std::shared_ptr<IConnector> lExpectedConnector{std::make_shared<IConnectorMock>()};
    PCapReader                  lPCapExpectedReader{false, false};


    pcap_t*           lHandler        = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
    const std::string lOutputFileName = "../Tests/Output/PromiscuousToMonitorOutput.pcap";
    pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());

    lConnector->Open("../Tests/Input/PromiscuousHelloWorld.pcapng");

    // This is a bit of a hack, pcapreader won't do any conversion if it thinks the output is promiscuous traffic
    std::vector<std::string> lSSIDFilter{"T#STNET"};
    lPCapExpectedReader.Open("../Tests/Input/PromiscuousToMonitorOutput_Expected.pcap", lSSIDFilter);

    lConnector->SetIncomingConnection(lIncomingConnection);
    lPCapExpectedReader.SetConnector(lExpectedConnector);

    std::vector<std::string> lSendBuffer{};
    std::vector<timeval>     lTimeStamp{};

    std::vector<std::string> lSendExpectedBuffer{};

    std::shared_ptr<Handler80211> lPacketHandler =
        std::dynamic_pointer_cast<Handler80211>(lPCapExpectedReader.GetPacketHandler());

    // Our expected output didn't contain any beacon frames, so tell the handler what to listen to.
    lPacketHandler->SetBSSID(MacToInt("01:23:45:67:ab:cd"));
    
    ASSERT_NE(lPacketHandler, nullptr);

    // This piece of magic will grab sent output for me and save its timestamps
    EXPECT_CALL(*std::dynamic_pointer_cast<IPCapDeviceMock>(lIncomingConnection), Send(_))
        .WillRepeatedly(DoAll(WithArg<0>([&](std::string_view aMessage) {
                                  lSendBuffer.emplace_back(aMessage);
                                  lTimeStamp.push_back(lConnector->GetHeader()->ts);
                              }),
                              Return(true)));

    EXPECT_CALL(*std::dynamic_pointer_cast<IConnectorMock>(lExpectedConnector), Send(_)).WillRepeatedly(Return(true));

    // Do not use thread here because we want to do some manual stuff
    while (lConnector->ReadNextData() && lPCapExpectedReader.ReadNextData()) {
        std::string lExpectedDataString{
            lPCapExpectedReader.DataToString(lPCapExpectedReader.GetData(), lPCapExpectedReader.GetHeader())};

        lSendExpectedBuffer.push_back(lExpectedDataString);
        
        // We are secretly going to update twice here, but this is so we can get the data parameters before
        // ReadCallBack is called
        lPacketHandler->Update(lExpectedDataString);
        
        lPCapExpectedReader.ReadCallback(lPCapExpectedReader.GetData(), lPCapExpectedReader.GetHeader());

        // Update parameters for the 80211 conversion
        lConnector->SetBSSID(lPacketHandler->GetLockedBSSID());
        auto lParameters{std::make_shared<RadioTapReader::PhysicalDeviceParameters>(lPacketHandler->GetDataPacketParameters())};
        lConnector->SetParameters(lParameters);

        // Now run the callback with correct parameters
        lConnector->ReadCallback(lConnector->GetData(), lConnector->GetHeader());
    }

    ASSERT_EQ(lSendBuffer.size(), lSendExpectedBuffer.size());

    size_t lCount = 0;
    for (auto& lMessage : lSendBuffer) {
        pcap_pkthdr lHeader{};
        lHeader.caplen = lMessage.size();
        lHeader.len    = lMessage.size();
        lHeader.ts     = lTimeStamp.at(lCount);

        // Output a file with the results as well so the results can be further inspected
        pcap_dump(reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const u_char*>(lMessage.c_str()));

        EXPECT_EQ(lMessage, lSendExpectedBuffer.at(lCount));
        lCount++;
    }

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lConnector->Close();
    lPCapExpectedReader.Close();
}



// TEST_F(PacketHandlingTest, ConstructAcknowledgementFrame)
// {
//     pcap_t*           lHandler        = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
//     const std::string lOutputFileName = "../Tests/Output/ConstructAcknowledgementFrame.pcap";
//     pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());

//     PCapReader               lPCapExpectedReader{};
//     std::vector<std::string> lFilter{"None"};
//     lPCapExpectedReader.Open("../Tests/Input/ConstructAcknowledgementFrame_Expected.pcap", lFilter, 2412);


//     // Create Acknowledgement frame
//     std::array<uint8_t, 6> lTransmitterAddress{0x01, 0x23, 0x45, 0x67, 0x89, 0xab};
//     std::string lAcknowledgementFrame{mPacketConverter.ConstructAcknowledgementFrame(lTransmitterAddress, 2412,
//     0x6c)};

//     pcap_pkthdr lAcknowledgementFrameHeader{};
//     lAcknowledgementFrameHeader.caplen = lAcknowledgementFrame.size();
//     lAcknowledgementFrameHeader.len    = lAcknowledgementFrame.size();

//     // Output to file for easy inspection
//     pcap_dump(reinterpret_cast<u_char*>(lDumper),
//               &lAcknowledgementFrameHeader,
//               reinterpret_cast<const u_char*>(lAcknowledgementFrame.c_str()));

//     // Now compare against expectation
//     ASSERT_TRUE(lPCapExpectedReader.ReadNextData());
//     ASSERT_EQ(lPCapExpectedReader.LastDataToString(), lAcknowledgementFrame);
// }