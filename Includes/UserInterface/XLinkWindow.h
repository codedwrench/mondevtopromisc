#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.h
 *
 * This file contains an class for a userinterface xlink window.
 *
 **/

#include "Window.h"

namespace XLinkWindow_Constants
{
    const std::string_view cQuitMessage{"Press q to quit"};
    const std::string_view cStartEngineMessage{"Start Engine"};
    const std::string_view cStopEngineMessage{"Stop Engine"};
    const std::string_view cSaveMessage{"Save config"};
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
};