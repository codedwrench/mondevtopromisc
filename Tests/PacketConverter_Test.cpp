/* Copyright (c) 2020 [Rick de Bondt] - PacketConverter_Test.cpp
 * This file contains tests for the PacketConverter class.
 *
 * */

#include "../Includes/PacketConverter.h"

#include <gtest/gtest.h>

#include "../Includes/PCapReader.h"

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
    lPCapReader.Open("../Tests/Input/PromiscuousHelloWorld.pcapng");
    lPCapExpectedReader.Open("../Tests/Input/PromiscuousToMonitorOutput_Expected.pcap");

    while (lPCapReader.ReadNextData()) {
        std::string lDataToConvert = lPCapReader.LastDataToString();
        lDataToConvert =
            mPacketConverter.ConvertPacketTo80211(lDataToConvert, mPacketConverter.MacToInt("01:23:45:67:AB:CD"));

        pcap_pkthdr lHeader{};
        lHeader.caplen = lDataToConvert.size();
        lHeader.len    = lDataToConvert.size();
        lHeader.ts     = lPCapReader.GetHeader()->ts;

        // Output a file with the results as well so the results can be further inspected
        pcap_dump((u_char*) lDumper, &lHeader, reinterpret_cast<const u_char*>(lDataToConvert.c_str()));

        // It should never be the case that there is no next packet available, then the expected output doesn't match.
        ASSERT_TRUE(lPCapExpectedReader.ReadNextData());

        ASSERT_EQ(lPCapExpectedReader.LastDataToString(), lDataToConvert);
    }

    // No new packets should be available on the expected output.
    ASSERT_FALSE(lPCapExpectedReader.ReadNextData());

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
}

TEST_F(PacketConverterTest, MonitorToPromiscuous)
{
    PCapReader        lPCapReader{};
    PCapReader        lPCapExpectedReader{};
    pcap_t*           lHandler        = pcap_open_dead(DLT_EN10MB, 65535);
    const std::string lOutputFileName = "../Tests/Output/MonitorToPromiscuousOutput.pcap";
    pcap_dumper_t*    lDumper         = pcap_dump_open(lHandler, lOutputFileName.c_str());
    lPCapReader.Open("../Tests/Input/MonitorHelloWorld.pcapng");
    lPCapExpectedReader.Open("../Tests/Input/MonitorToPromiscuousOutput_Expected.pcap");

    while (lPCapReader.ReadNextData()) {
        std::string lDataToConvert = lPCapReader.LastDataToString();
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
            ASSERT_TRUE(lPCapExpectedReader.ReadNextData());

            ASSERT_EQ(lPCapExpectedReader.LastDataToString(), lDataToConvert);
        }
    }

    // No new packets should be available on the expected output.
    ASSERT_FALSE(lPCapExpectedReader.ReadNextData());

    pcap_dump_close(lDumper);
    pcap_close(lHandler);

    lPCapReader.Close();
    lPCapExpectedReader.Close();
}
