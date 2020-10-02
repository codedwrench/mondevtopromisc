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
    XLinkWindow(WindowModel&                                                     aModel,
                std::string_view                                                 aTitle,
                const std::function<std::array<int, 4>(const int&, const int&)>& aScaleCalculation,
                const int&                                                       aMaxHeight,
                const int&                                                       aMaxWidth);

    void SetUp() override;
    void Draw() override;
};