#include "../../Includes/UserInterface/XLinkWindow.h"

#include <cmath>

#include "../../Includes/Logger.h"
#include "../../Includes/UserInterface/CheckBox.h"
#include "../../Includes/UserInterface/String.h"

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.cpp */

const std::string_view cQuitMessage{"Press q to quit"};

std::array<int, 4> SearchXLinkCheckBoxScaleFunction(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {2, 2, 0, 0};
}

std::array<int, 4> TabPressStringScaleFunction(const int& aMaxHeight, const int& /*aMaxWidth*/)
{
    return {aMaxHeight - 1, 1, 0, 0};
}

std::array<int, 4> QQuitStringScaleFunction(const int& aMaxHeight, const int& aMaxWidth)
{
    return {aMaxHeight - 1, static_cast<int>(aMaxWidth - 1 - cQuitMessage.length()), 0, 0};
}

XLinkWindow::XLinkWindow(WindowModel&                                                     aModel,
                         std::string_view                                                 aTitle,
                         const std::function<std::array<int, 4>(const int&, const int&)>& aScaleCalculation,
                         const int&                                                       aMaxHeight,
                         const int&                                                       aMaxWidth) :
    Window(aModel, aTitle, aScaleCalculation, aMaxHeight, aMaxWidth)
{
    SetUp();
}

void XLinkWindow::SetUp()
{
    Window::SetUp();
    // Get size of window so scaling works properly.
    GetSize();

    AddObject(std::make_unique<CheckBox>(*this,
                                         "Automatically search for XLink Kai instances",
                                         SearchXLinkCheckBoxScaleFunction,
                                         GetHeightReference(),
                                         GetWidthReference(),
                                         GetModel().mAutoDiscoverXLinkKaiInstance));
    AddObject(std::make_unique<String>(
        *this, "Press Tab to switch panes", TabPressStringScaleFunction, GetHeightReference(), GetWidthReference()));
    AddObject(std::make_unique<String>(
        *this, cQuitMessage, QQuitStringScaleFunction, GetHeightReference(), GetWidthReference()));
}

void XLinkWindow::Draw()
{
    Window::Draw();
}