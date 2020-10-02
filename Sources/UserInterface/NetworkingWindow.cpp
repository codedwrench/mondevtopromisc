#include "../../Includes/UserInterface/NetworkingWindow.h"

#include <cmath>
#include <utility>

#include "../../Includes/UserInterface/CheckBox.h"

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingWindow.cpp */

const std::string_view cQuitMessage{"Press q to quit"};

Dimensions ScaleSearchPSPNetworks(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {2, 2, 0, 0};
}

Dimensions ScaleSearchTakeHintsXLinkKai(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {3, 2, 0, 0};
}


NetworkingWindow::NetworkingWindow(WindowModel&                aModel,
                                   std::string_view            aTitle,
                                   std::function<Dimensions()> aCalculation) :
    Window(aModel, aTitle, std::move(aCalculation))
{
    SetUp();
}

void NetworkingWindow::SetUp()
{
    Window::SetUp();

    AddObject(std::make_unique<CheckBox>(
        *this,
        "Automatically connect to PSP/Vita networks.",
        [&] { return ScaleSearchPSPNetworks(GetHeightReference(), GetWidthReference()); },
        GetModel().mAutoDiscoverNetworks));
    AddObject(std::make_unique<CheckBox>(
        *this,
        "Take hints from XLink Kai.",
        [&] { return ScaleSearchTakeHintsXLinkKai(GetHeightReference(), GetWidthReference()); },
        GetModel().mXLinkKaiHints));
}