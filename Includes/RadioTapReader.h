#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - RadioTapReader.h
 *
 * This file contains the necessary components to dissect a radiotap header and return the nessecary information.
 *
 **/

#include <string_view>

class RadioTapReader
{
public:
    void     FillRadioTapParameters(std::string_view aData);
    uint16_t GetLength(std::string_view aData);
    uint8_t  GetFlags(std::string_view aData);
    uint8_t  GetPresentFlags(std::string_view aData);
    uint8_t  GetDataRate(std::string_view aData);
    uint16_t GetFrequency(std::string_view aData);
    uint16_t GetChannelFlags(std::string_view aData);

private:
    uint16_t mLength{0};
    uint32_t mPresentFlags{0};
    uint8_t  mFlags{0};
    uint8_t  mDataRate{0};
    uint16_t mFrequency{2412};
    uint16_t mChannelFlags{0};
};
