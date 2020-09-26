#include "../../Includes/UserInterface/NetworkingWindow.h"

#include <cmath>

#include "../../Includes/UserInterface/CheckBox.h"

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingWindow.cpp */

NetworkingWindow::NetworkingWindow(
    std::string_view aTitle, int aYCoord, int aXCoord, int aLines, int aColumns, bool aExclusive, bool aVisible) :
    Window(aTitle, aYCoord, aXCoord, aLines, aColumns, aExclusive, aVisible)
{
    SetUp();
}

void NetworkingWindow::SetUp()
{
    Window::SetUp();
    AddObject(std::make_unique<CheckBox>(*this, "Automatically connect to PSP/Vita networks.", 2, 2));
    AddObject(std::make_unique<CheckBox>(*this, "Take hints from XLink Kai.", 3, 2));
}

bool NetworkingWindow::Scale(int aMaxHeight, int aMaxWidth)
{
    return Resize(floor(aMaxHeight / 2.0), aMaxWidth);
}