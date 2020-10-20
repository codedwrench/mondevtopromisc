/* Copyright (c) 2020 [Rick de Bondt] - PacketConverter_Test.cpp
 * This file contains tests for the PacketConverter class.
 **/

#include "../Includes/PacketConverter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Includes/PCapReader.h"

class ISendReceiveDeviceMock : public ISendReceiveDevice
{
public:
    MOCK_METHOD(void, Close, ());
    MOCK_METHOD(std::string, LastDataToString, ());
    MOCK_METHOD(bool, ReadNextData, ());
    MOCK_METHOD(bool, Send, (std::string_view aData));
    MOCK_METHOD(void, SetSendReceiveDevice, (std::shared_ptr<ISendReceiveDevice> aDevice));
};
class PacketConverterTest : public ::testing::Test
{
protected:
    PacketConverter mPacketConverter{true};
};

// Tests whether MacToInt can successfully convert a well formed MAC address string.
TEST_F(PacketConverterTest, MacToInt)
{
    uint64_t aResult{mPacketConverter.MacToInt("01:23:45:67:AB:CD")};
    ASSERT_EQ(aResult, 0x01234567abcd);
}

TEST_F(PacketConverterTest, PromiscuousToMonitor)
{
    PCapReader        lPCapReader{};
    PCapReader        lPCapExpectedReader{};
    pcap_t*           lHandler        = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
    const std::string lOutputFileName = "../Tests/Output/PromiscuousToMonitorOutput.pcap";
    pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());
    lPCapReader.Open("../Tests/Input/PromiscuousHelloWorld.pcapng", 2412);

    // We're manually entering the BSSID, so we don't have to match it or anything.
    std::vector<std::string> lSSIDFilter{"None"};
    lPCapExpectedReader.Open("../Tests/Input/PromiscuousToMonitorOutput_Expected.pcap", lSSIDFilter, 2412);

    while (lPCapReader.ReadNextData()) {
        std::string lDataToConvert = lPCapReader.LastDataToString();
        lDataToConvert             = mPacketConverter.ConvertPacketTo80211(lDataToConvert,
                                                               mPacketConverter.MacToInt("01:23:45:67:AB:CD"),
                                                               RadioTap_Constants::cChannel,
                                                               RadioTap_Constants::cRateFlags);

        pcap_pkthdr lHeader{};
        lHeader.caplen = lDataToConvert.size();
        lHeader.len    = lDataToConvert.size();
        lHeader.ts     = lPCapReader.GetHeader()->ts;

        // Output a file with the results as well so the results can be further inspected
        pcap_dump(
            reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const u_char*>(lDataToConvert.c_str()));

        // It should never be the case that there is no next packet available, then the expected output doesn't match.

    }

    // No new packets should be available on the expected output.

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
}

TEST_F(PacketConverterTest, MonitorToPromiscuous)
{
    PCapReader               lPCapReader{};
    PCapReader               lPCapExpectedReader{};
    pcap_t*                  lHandler        = pcap_open_dead(DLT_EN10MB, 65535);
    const std::string        lOutputFileName = "../Tests/Output/MonitorToPromiscuousOutput.pcap";
    pcap_dumper_t*           lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());
    std::vector<std::string> lSSIDFilter{"None"};
    lPCapReader.Open("../Tests/Input/MonitorHelloWorld.pcapng", lSSIDFilter, 2412);
    lPCapExpectedReader.Open("../Tests/Input/MonitorToPromiscuousOutput_Expected.pcap", 2412);

    while (lPCapReader.ReadNextData()) {
        std::string lDataToConvert = lPCapReader.LastDataToString();
        mPacketConverter.Update(lDataToConvert);
        if (mPacketConverter.Is80211Data(lDataToConvert) &&
            mPacketConverter.IsForBSSID(lDataToConvert, mPacketConverter.MacToInt("62:58:c5:07:95:5e"))) {
            lDataToConvert = mPacketConverter.ConvertPacketTo8023(lDataToConvert);

            pcap_pkthdr lHeader{};
            lHeader.caplen = lDataToConvert.size();
            lHeader.len    = lDataToConvert.size();
            lHeader.ts     = lPCapReader.GetHeader()->ts;

            // Output a file with the results as well so the results can be further inspected
            pcap_dump(
                reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const u_char*>(lDataToConvert.c_str()));

            // It should never be the case that there is no next packet available, then the expected output doesn't
            // match.

        }
    }

    // No new packets should be available on the expected output.

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
}

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
TEST_F(PacketConverterTest, CopyBeaconInformation)
{
    std::shared_ptr<ISendReceiveDevice> lSendReceiveDeviceMock{std::make_shared<ISendReceiveDeviceMock>()};
    PCapReader                          lPCapReader{};
    PCapReader                          lPCapExpectedReader{};
    pcap_t*                             lHandler        = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
    const std::string                   lOutputFileName = "../Tests/Output/MonitorBeaconChange.pcap";
    pcap_dumper_t*                      lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());

    std::vector<std::string> lSSIDFilter{"T#STNET"};
    lPCapReader.Open("../Tests/Input/MonitorHelloWorld.pcapng", lSSIDFilter, 2412);
    lPCapReader.SetSendReceiveDevice(lSendReceiveDeviceMock);
    lPCapExpectedReader.Open("../Tests/Input/MonitorBeaconChange_Expected.pcap", lSSIDFilter, 2412);


    std::string lSendBuffer{};

    EXPECT_CALL(*std::dynamic_pointer_cast<ISendReceiveDeviceMock>(lSendReceiveDeviceMock), Send(_))
        .WillRepeatedly(DoAll(SaveArg<0>(&lSendBuffer), Return(true)));
    while (lPCapReader.ReadNextData()) {
        // Convert the packet and "Send" it so we get the converted information
        mPacketConverter.Update(lPCapReader.LastDataToString());
        std::pair<bool, bool> lSuccessfulAndUseful{lPCapReader.ConstructAndReplayPacket(
            lPCapReader.GetData(), lPCapReader.GetHeader(), mPacketConverter, true)};

        if (lSuccessfulAndUseful.first && lSuccessfulAndUseful.second) {
            IPCapDevice_Constants::WiFiBeaconInformation lBeaconInformation{lPCapReader.GetWifiInformation()};

            // So from 802.3 straight back to 802.11, hopefully losing as little information as possible.
            std::string lDataToConvert = mPacketConverter.ConvertPacketTo80211(
                lSendBuffer, lBeaconInformation.BSSID, lBeaconInformation.Frequency, lBeaconInformation.MaxRate);

            // Save the file for easy inspection
            pcap_pkthdr lHeader{};
            lHeader.caplen = lDataToConvert.size();
            lHeader.len    = lDataToConvert.size();
            lHeader.ts     = lPCapReader.GetHeader()->ts;

            pcap_dump(
                reinterpret_cast<u_char*>(lDumper), &lHeader, reinterpret_cast<const u_char*>(lDataToConvert.c_str()));

            // It should never be the case that there is no next packet available, then the expected output doesn't
            // match.

        }
    }

    // No new packets should be available on the expected output.

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
}

TEST_F(PacketConverterTest, ConstructAcknowledgementFrame)
{
    pcap_t*           lHandler        = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
    const std::string lOutputFileName = "../Tests/Output/ConstructAcknowledgementFrame.pcap";
    pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());

    PCapReader               lPCapExpectedReader{};
    std::vector<std::string> lFilter{"None"};
    lPCapExpectedReader.Open("../Tests/Input/ConstructAcknowledgementFrame_Expected.pcap", lFilter, 2412);


    // Create Acknowledgement frame
    std::array<uint8_t, 6> lTransmitterAddress{0x01, 0x23, 0x45, 0x67, 0x89, 0xab};
    std::string lAcknowledgementFrame{mPacketConverter.ConstructAcknowledgementFrame(lTransmitterAddress, 2412, 0x6c)};

    pcap_pkthdr lAcknowledgementFrameHeader{};
    lAcknowledgementFrameHeader.caplen = lAcknowledgementFrame.size();
    lAcknowledgementFrameHeader.len    = lAcknowledgementFrame.size();

    // Output to file for easy inspection
    pcap_dump(reinterpret_cast<u_char*>(lDumper),
              &lAcknowledgementFrameHeader,
              reinterpret_cast<const u_char*>(lAcknowledgementFrame.c_str()));

    // Now compare against expectation
}