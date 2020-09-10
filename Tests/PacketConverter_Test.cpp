/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains tests for the PacketConverter class.
 *
 * */

#include "../Includes/PacketConverter.h"

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

TEST_F(PacketConverterTest, MonitorToPromiscuous)
{
    EXPECT_EQ(2, 1 + 1);
}

TEST_F(PacketConverterTest, PromiscuousToMonitor)
{
    EXPECT_EQ(2, 1 + 1);
}
