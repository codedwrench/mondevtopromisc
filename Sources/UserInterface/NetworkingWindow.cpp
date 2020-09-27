#include "../../Includes/UserInterface/NetworkingWindow.h"

#include <cmath>

#include "../../Includes/UserInterface/CheckBox.h"

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingWindow.cpp */

NetworkingWindow::NetworkingWindow(std::string_view                                                 aTitle,
                                   const std::function<std::array<int, 4>(const int&, const int&)>& aScaleCalculation,
                                   const int&                                                       aMaxHeight,
                                   const int&                                                       aMaxWidth) :
    Window(aTitle, aScaleCalculation, aMaxHeight, aMaxWidth)
{
    SetUp();
}

void NetworkingWindow::SetUp()
{
    Window::SetUp();
    AddObject(std::make_unique<CheckBox>(*this, "Automatically connect to PSP/Vita networks.", 2, 2));
    AddObject(std::make_unique<CheckBox>(*this, "Take hints from XLink Kai.", 3, 2));
}