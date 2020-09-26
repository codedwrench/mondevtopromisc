#include "../../Includes/UserInterface/XLinkWindow.h"

#include "../../Includes/UserInterface/CheckBox.h"
#include "../../Includes/UserInterface/String.h"

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.cpp */

XLinkWindow::XLinkWindow(
    std::string_view aTitle, int aYCoord, int aXCoord, int aLines, int aColumns, bool aExclusive, bool aVisible) :
    Window(aTitle, aYCoord, aXCoord, aLines, aColumns, aExclusive, aVisible)
{
    SetUp();
}

void XLinkWindow::SetUp()
{
    Window::SetUp();
    std::pair<int, int> lWindowSize = GetSize();
    AddObject(std::make_unique<CheckBox>(*this, "Automatically search for XLink Kai instances.", 2, 2));
    AddObject(std::make_unique<String>(*this, "Press Tab to switch panes", lWindowSize.first - 1, 1));
    std::string lQuitMessage{"Press q to quit"};
    AddObject(std::make_unique<String>(*this, lQuitMessage, lWindowSize.first - 1, lWindowSize.second - lQuitMessage.length() - 1));
}

void XLinkWindow::Draw()
{
    Window::Draw();
}