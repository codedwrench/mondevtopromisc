/* Copyright (c) 2020 [Rick de Bondt] - PacketConverter_Test.cpp
 * This file contains tests for the PacketConverter class.
 **/


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Includes/NetConversionFunctions.h"
#include "../Includes/PCapReader.h"


using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::WithArg;
class IConnectorMock : public IConnector
{
public:
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(std::string, LastDataToString, ());
    MOCK_METHOD(bool, ReadNextData, ());
    MOCK_METHOD(bool, Send, (std::string_view aData));
    MOCK_METHOD(void, SetIncomingConnection, (std::shared_ptr<IPCapDevice> aDevice));
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
    std::vector<std::string> lDummySSIDFilter{"None"};
    lPCapExpectedReader.Open("../Tests/Input/MonitorToPromiscuousOutput_Expected.pcap", lDummySSIDFilter);

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

// TEST_F(PacketHandlingTest, MonitorToPromiscuous)
// {
//     PCapReader               lPCapReader{};
//     PCapReader               lPCapExpectedReader{};
//     pcap_t*                  lHandler        = pcap_open_dead(DLT_EN10MB, 65535);
//     const std::string        lOutputFileName = "../Tests/Output/MonitorToPromiscuousOutput.pcap";
//     pcap_dumper_t*           lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());
//     std::vector<std::string> lSSIDFilter{"None"};
//     lPCapReader.Open("../Tests/Input/MonitorHelloWorld.pcapng", lSSIDFilter, 2412);
//     lPCapExpectedReader.Open("../Tests/Input/MonitorToPromiscuousOutput_Expected.pcap", 2412);

//     while (lPCapReader.ReadNextData()) {
//         std::string lDataToConvert = lPCapReader.LastDataToString();
//         mPacketConverter.Update(lDataToConvert);
//         if (mPacketConverter.Is80211Data(lDataToConvert) &&
//             mPacketConverter.IsForBSSID(lDataToConvert, mPacketConverter.MacToInt("62:58:c5:07:95:5e"))) {
//             lDataToConvert = mPacketConverter.ConvertPacketTo8023(lDataToConvert);

//             pcap_pkthdr lHeader{};
//             lHeader.caplen = lDataToConvert.size();
//             lHeader.len    = lDataToConvert.size();
//             lHeader.ts     = lPCapReader.GetHeader()->ts;

//             // Output a file with the results as well so the results can be further inspected
//             pcap_dump(
//                 reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const
//                 u_char*>(lDataToConvert.c_str()));

//             // It should never be the case that there is no next packet available, then the expected output doesn't
//             // match.
//             ASSERT_TRUE(lPCapExpectedReader.ReadNextData());

//             ASSERT_EQ(lPCapExpectedReader.LastDataToString(), lDataToConvert);
//         }
//     }

//     // No new packets should be available on the expected output.
//     ASSERT_FALSE(lPCapExpectedReader.ReadNextData());

//     pcap_dump_close(lDumper);
//     pcap_close(lHandler);

//     lPCapReader.Close();
//     lPCapExpectedReader.Close();
// }

// TEST_F(PacketHandlingTest, CopyBeaconInformation)
// {
//     std::shared_ptr<IConnector> lSendReceiveDeviceMock{std::make_shared<IConnectorMock>()};
//     PCapReader                  lPCapReader{};
//     PCapReader                  lPCapExpectedReader{};
//     pcap_t*                     lHandler        = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
//     const std::string           lOutputFileName = "../Tests/Output/MonitorBeaconChange.pcap";
//     pcap_dumper_t*              lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());

//     std::vector<std::string> lSSIDFilter{"T#STNET"};
//     lPCapReader.Open("../Tests/Input/MonitorHelloWorld.pcapng", lSSIDFilter, 2412);
//     lPCapReader.SetSendReceiveDevice(lSendReceiveDeviceMock);
//     lPCapExpectedReader.Open("../Tests/Input/MonitorBeaconChange_Expected.pcap", lSSIDFilter, 2412);


//     std::string lSendBuffer{};

//     EXPECT_CALL(*std::dynamic_pointer_cast<IConnectorMock>(lSendReceiveDeviceMock), Send(_))
//         .WillRepeatedly(DoAll(SaveArg<0>(&lSendBuffer), Return(true)));
//     while (lPCapReader.ReadNextData()) {
//         // Convert the packet and "Send" it so we get the converted information
//         mPacketConverter.Update(lPCapReader.LastDataToString());
//         std::pair<bool, bool> lSuccessfulAndUseful{lPCapReader.ConstructAndReplayPacket(
//             lPCapReader.GetData(), lPCapReader.GetHeader(), mPacketConverter, true)};

//         if (lSuccessfulAndUseful.first && lSuccessfulAndUseful.second) {
//             IPCapDevice_Constants::WiFiBeaconInformation lBeaconInformation{lPCapReader.GetWifiInformation()};

//             // So from 802.3 straight back to 802.11, hopefully losing as little information as possible.
//             std::string lDataToConvert = mPacketConverter.ConvertPacketTo80211(
//                 lSendBuffer, lBeaconInformation.BSSID, lBeaconInformation.Frequency, lBeaconInformation.MaxRate);

//             // Save the file for easy inspection
//             pcap_pkthdr lHeader{};
//             lHeader.caplen = lDataToConvert.size();
//             lHeader.len    = lDataToConvert.size();
//             lHeader.ts     = lPCapReader.GetHeader()->ts;

//             pcap_dump(
//                 reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const
//                 u_char*>(lDataToConvert.c_str()));

//             // It should never be the case that there is no next packet available, then the expected output doesn't
//             // match.
//             ASSERT_TRUE(lPCapExpectedReader.ReadNextData());

//             ASSERT_EQ(lPCapExpectedReader.LastDataToString(), lDataToConvert);
//         }
//     }

//     // No new packets should be available on the expected output.
//     ASSERT_FALSE(lPCapExpectedReader.ReadNextData());

//     pcap_dump_close(lDumper);
//     pcap_close(lHandler);

//     lPCapReader.Close();
//     lPCapExpectedReader.Close();
// }

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