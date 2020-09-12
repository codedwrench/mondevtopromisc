/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
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
    pcap_t*           lHandler       = pcap_open_dead(DLT_IEEE802_11_RADIO, 65535);
    const std::string lOutputFileName = "../Tests/Output/PromisciousToMonitorOutput.pcap";
    pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());
    lPCapReader.Open("../Tests/Input/promisc_hello_world.pcapng");

    while(lPCapReader.ReadNextPacket()) {
        std::string lDataToConvert = lPCapReader.DataToString();
        lDataToConvert = mPacketConverter.ConvertPacketTo80211(lDataToConvert, "01:23:45:67:AB:CD");

        pcap_pkthdr lHeader;
        lHeader.caplen = lDataToConvert.size();
        lHeader.len = lDataToConvert.size();

        pcap_dump((u_char*) lDumper, &lHeader, reinterpret_cast<const u_char*>(lDataToConvert.c_str()));

    }
    pcap_dump_close(lDumper);
    pcap_close(lHandler);
}

TEST_F(PacketConverterTest, MonitorToPromiscuous)
{
    EXPECT_EQ(2, 1 + 1);
}
