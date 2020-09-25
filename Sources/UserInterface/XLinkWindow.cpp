#include "../../Includes/UserInterface/XLinkWindow.h"

#include "../../Includes/UserInterface/CheckBox.h"

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
    AddObject(std::make_unique<CheckBox>(*this, "Auto discover", 2, 2, false, false));
}