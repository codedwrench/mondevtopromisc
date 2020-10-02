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
    NetworkingWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation);

    void SetUp() override;
};