#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingWindow.h
 *
 * This file contains an class for a userinterface networking window.
 *
 **/

#include "Window.h"

/**
 * Class that will setup and draw a networking window.
 **/
class NetworkingWindow : public Window
{
public:
    NetworkingWindow(std::string_view aTitle,
                int              aYCoord,
                int              aXCoord,
                int              aLines,
                int              aColumns,
                bool             aExclusive = false,
                bool             aVisible   = true);

    void SetUp() override;

    bool Scale(int aMaxHeight, int aMaxWidth) override;
};