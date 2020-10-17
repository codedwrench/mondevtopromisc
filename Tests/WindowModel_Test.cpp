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

// Tests whether MacToInt can successfully convert a well formed MAC address string.
TEST_F(WindowModelTest, SaveModel)
{
    ASSERT_TRUE(mWindowModel.SaveToFile("../Tests/Output/config.txt"));
    std::ifstream lOutputFile{};
    std::ifstream lExpectedFile{};

    lOutputFile.open("../Tests/Output/config.txt");
    lExpectedFile.open("../Tests/Input/config_expected.txt");

    ASSERT_TRUE(lOutputFile.is_open() && lExpectedFile.is_open());

    std::string lOutputLine{};
    std::string lExpectedLine{};
    while ((!lOutputFile.eof()) || (!lExpectedFile.eof())) {
        getline(lOutputFile, lExpectedLine);
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
