#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingWindow.h
 *
 * This file contains an class for a userinterface networking window.
 *
 **/

#include "Window.h"

namespace
{
    static constexpr std::string_view cWifiAdapterToUse{"WiFi adapter to use"};
    static constexpr std::string_view cChannel{"WiFi channel to listen to"};
    static constexpr std::string_view cScanWifiNetworksPSP{"Automatically connect to PSP/Vita networks"};
    static constexpr std::string_view cEnableAcknowledgeDataFrames{"EXPERIMENTAL: Acknowledge data frames"};
    static constexpr std::string_view cTakeHintsFromXlinkKai{"Take hints from XLink Kai."};
}  // namespace

/**
 * Class that will setup and draw a networking window.
 **/
class NetworkingWindow : public Window
{
public:
    NetworkingWindow(WindowModel& aModel, std::string_view aTitle, const std::function<Dimensions()>& aCalculation);

    void SetUp() override;
};