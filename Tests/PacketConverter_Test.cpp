/* Copyright (c) 2020 [Rick de Bondt] - PacketConverter_Test.cpp
 * This file contains tests for the PacketConverter class.
 *
 * */

#include "../Includes/PacketConverter.h"
#include "../Includes/PCapReader.h"

#include <gtest/gtest.h>

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
    PCapReader lPCapReader{};
    PCapReader lPCapExpectedReader{};
    pcap_t*           lHandler       = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
    const std::string lOutputFileName = "../Tests/Output/PromiscuousToMonitorOutput.pcap";
    pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());
    lPCapReader.Open("../Tests/Input/PromiscuousHelloWorld.pcapng");
    lPCapExpectedReader.Open("../Tests/Input/PromiscuousToMonitorOutput_Expected.pcap");

    while(lPCapReader.ReadNextPacket()) {
        std::string lDataToConvert = lPCapReader.DataToString();
        lDataToConvert = mPacketConverter.ConvertPacketTo80211(lDataToConvert, "01:23:45:67:AB:CD");

        pcap_pkthdr lHeader;
        lHeader.caplen = lDataToConvert.size();
        lHeader.len = lDataToConvert.size();

        // Output a file with the results as well so the results can be further inspected
        pcap_dump((u_char*) lDumper, &lHeader, reinterpret_cast<const u_char*>(lDataToConvert.c_str()));

        // It should never be the case that there is no next packet available, then the expected output doesn't match.
        ASSERT_TRUE(lPCapExpectedReader.ReadNextPacket());

        ASSERT_EQ(lPCapExpectedReader.DataToString(), lDataToConvert);
    }
    pcap_dump_close(lDumper);
    pcap_close(lHandler);
}

TEST_F(PacketConverterTest, MonitorToPromiscuous)
{
    EXPECT_EQ(2, 1 + 1);
}
