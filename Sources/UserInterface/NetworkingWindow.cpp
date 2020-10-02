#include "../../Includes/UserInterface/NetworkingWindow.h"

#include <cmath>

#include "../../Includes/UserInterface/CheckBox.h"

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingWindow.cpp */

const std::string_view cQuitMessage{"Press q to quit"};

std::array<int, 4> SearchPSPNetworksScaleFunction(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {2, 2, 0, 0};
}

std::array<int, 4> SearchTakeHintsXLinkKaiScaleFunction(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {3, 2, 0, 0};
}


NetworkingWindow::NetworkingWindow(WindowModel&                                                     aModel,
                                   std::string_view                                                 aTitle,
                                   const std::function<std::array<int, 4>(const int&, const int&)>& aScaleCalculation,
                                   const int&                                                       aMaxHeight,
                                   const int&                                                       aMaxWidth) :
    Window(aModel, aTitle, aScaleCalculation, aMaxHeight, aMaxWidth)
{
    SetUp();
}

void NetworkingWindow::SetUp()
{
    Window::SetUp();

    AddObject(std::make_unique<CheckBox>(*this,
                                         "Automatically connect to PSP/Vita networks.",
                                         SearchPSPNetworksScaleFunction,
                                         GetHeightReference(),
                                         GetWidthReference(),
                                         GetModel().mAutoDiscoverNetworks));
    AddObject(std::make_unique<CheckBox>(*this,
                                         "Take hints from XLink Kai.",
                                         SearchTakeHintsXLinkKaiScaleFunction,
                                         GetHeightReference(),
                                         GetWidthReference(),
                                         GetModel().mXLinkKaiHints));
}