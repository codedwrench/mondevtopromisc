#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.h
 *
 * This file contains an class for a userinterface xlink window.
 *
 **/

#include "Window.h"


/**
 * Class that will setup and draw an XLink window.
 **/
class XLinkWindow : public Window
{
public:
    XLinkWindow(std::string_view aTitle,
                int              aYCoord,
                int              aXCoord,
                int              aLines,
                int              aColumns,
                bool             aExclusive = false,
                bool             aVisible   = true);

    void SetUp() override;
    void Draw() override;
};