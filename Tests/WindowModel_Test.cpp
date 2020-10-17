/* Copyright (c) 2020 [Rick de Bondt] - WindowModel_Test.cpp
 * This file contains tests for loading or saving WindowModel members.
 **/

#include "../Includes/WindowModel.h"

#include <fstream>

#include <gtest/gtest.h>

class WindowModelTest : public ::testing::Test
{
public:
    WindowModel mWindowModel{};
};

// Tests whether Window model data can be saved successfully
TEST_F(WindowModelTest, SaveModel)
{
    ASSERT_TRUE(mWindowModel.SaveToFile("../Tests/Output/config.txt"));
    std::ifstream lOutputFile;
    std::ifstream lExpectedFile;

    lOutputFile.open("../Tests/Output/config.txt");
    lExpectedFile.open("../Tests/Input/config_expected.txt");

    ASSERT_TRUE(lOutputFile.is_open() && lExpectedFile.is_open());

    std::string lOutputLine{};
    std::string lExpectedLine{};
    while ((!lOutputFile.eof()) || (!lExpectedFile.eof())) {
        getline(lOutputFile, lOutputLine);
        ASSERT_FALSE(lOutputFile.bad());
        // If failbit is set, eof bit should not be set, otherwise there is a real failure
        ASSERT_FALSE((!lOutputFile.eof()) && lOutputFile.fail());

        getline(lExpectedFile, lExpectedLine);
        ASSERT_FALSE(lExpectedFile.bad());
        // If failbit is set, eof bit should not be set, otherwise there is a real failure
        ASSERT_FALSE((!lExpectedFile.eof()) && lExpectedFile.fail());

        ASSERT_EQ(lExpectedLine, lOutputLine);
    }

    // Same amount of lines
    ASSERT_TRUE(lOutputFile.eof() && lExpectedFile.eof());
}

TEST_F(WindowModelTest, LoadModel)
{
    ASSERT_TRUE(mWindowModel.LoadFromFile("../Tests/Input/config_expected.txt"));

    // Check if those match
    EXPECT_EQ(mWindowModel.mLogLevel, WindowModel_Constants::cDefaultLogLevel);
    EXPECT_EQ(mWindowModel.mAutoDiscoverPSPVitaNetworks, WindowModel_Constants::cDefaultAutoDiscoverPSPVita);
    EXPECT_EQ(mWindowModel.mAutoDiscoverXLinkKaiInstance, WindowModel_Constants::cDefaultAutoDiscoverXLinkKai);
    EXPECT_EQ(mWindowModel.mXLinkKaiHints, WindowModel_Constants::cDefaultUseXLinkKaiHints);
    EXPECT_EQ(mWindowModel.mChannel, WindowModel_Constants::cDefaultChannel);
    EXPECT_EQ(mWindowModel.mWifiAdapter, WindowModel_Constants::cDefaultWifiAdapter);
    EXPECT_EQ(mWindowModel.mXLinkIp, WindowModel_Constants::cDefaultXLinkIp);
    EXPECT_EQ(mWindowModel.mXLinkPort, WindowModel_Constants::cDefaultXLinkPort);
}