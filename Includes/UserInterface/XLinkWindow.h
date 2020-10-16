#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.h
 *
 * This file contains an class for a userinterface xlink window.
 *
 **/

#include "Window.h"

namespace XLinkWindow_Constants
{
    static constexpr std::string_view cAutoSearchXLinkInstancesMessage{"Automatically search for XLink Kai instances"};
    static constexpr std::string_view cIPAddressMessage{"XLink IP address"};
    static constexpr std::string_view cPortMessage{"XLink Port"};
    static constexpr std::string_view cTabMessage{"Press Tab to switch panes"};
    static constexpr std::string_view cQuitMessage{"Press q to quit"};
    static constexpr std::string_view cStartEngineMessage{"Start Engine"};
    static constexpr std::string_view cStopEngineMessage{"Stop Engine"};
    static constexpr std::string_view cSaveMessage{"Save config"};
    static constexpr std::string_view cStatusPrefix{" Status: "};
    static constexpr std::string_view cDefaultStatusMessage{"Idle"};
}  // namespace XLinkWindow_Constants

using namespace XLinkWindow_Constants;

/**
 * Class that will setup and draw an XLink window.
 **/
class XLinkWindow : public Window
{
public:
    XLinkWindow(WindowModel& aModel, std::string_view aTitle, const std::function<Dimensions()>& aCalculation);

    void SetUp() override;
    void Draw() override;

private:
    WindowModel_Constants::EngineStatus mOldEngineStatus;
    Dimensions                          ScaleStatusMessage(const int& aMaxHeight, const int& aMaxWidth);
};