#include "../../Includes/UserInterface/XLinkWindow.h"

#include <cmath>

#include "../../Includes/Logger.h"
#include "../../Includes/UserInterface/CheckBox.h"
#include "../../Includes/UserInterface/String.h"

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.cpp */

XLinkWindow::XLinkWindow(std::string_view                                                 aTitle,
                         const std::function<std::array<int, 4>(const int&, const int&)>& aScaleCalculation,
                         const int&                                                       aMaxHeight,
                         const int&                                                       aMaxWidth) :
    Window(aTitle, aScaleCalculation, aMaxHeight, aMaxWidth)
{
    SetUp();
}

void XLinkWindow::SetUp()
{
    Window::SetUp();
    std::pair<int, int> lWindowSize = GetSize();
    AddObject(std::make_unique<CheckBox>(*this, "Automatically search for XLink Kai instances", 2, 2));
    AddObject(std::make_unique<String>(*this, "Press Tab to switch panes", lWindowSize.first - 1, 1));
    std::string lQuitMessage{"Press q to quit"};
    AddObject(std::make_unique<String>(
        *this, lQuitMessage, lWindowSize.first - 1, lWindowSize.second - lQuitMessage.length() - 1));
}

void XLinkWindow::Draw()
{
    Window::Draw();
}