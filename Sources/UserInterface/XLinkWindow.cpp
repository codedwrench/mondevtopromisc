#include "../../Includes/UserInterface/XLinkWindow.h"

#include <utility>

#include "../../Includes/UserInterface/Button.h"
#include "../../Includes/UserInterface/CheckBox.h"
#include "../../Includes/UserInterface/String.h"

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.cpp */

Dimensions ScaleSearchXLinkCheckBox(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {2, 2, 0, 0};
}

Dimensions ScaleTabPressString(const int& aMaxHeight, const int& /*aMaxWidth*/)
{
    return {aMaxHeight - 1, 1, 0, 0};
}

Dimensions ScaleQQuitString(const int& aMaxHeight, const int& aMaxWidth)
{
    return {aMaxHeight - 1, static_cast<int>(aMaxWidth - 1 - cQuitMessage.length()), 0, 0};
}

Dimensions ScaleSaveButton(const int& aMaxHeight, const int& aMaxWidth)
{
    return {
        aMaxHeight - 2, static_cast<int>(aMaxWidth - 10 - cStartEngineMessage.length() - cSaveMessage.length()), 0, 0};
}

Dimensions ScaleStartEngine(const int& aMaxHeight, const int& aMaxWidth)
{
    return {aMaxHeight - 2, static_cast<int>(aMaxWidth - 5 - cStartEngineMessage.length()), 0, 0};
}

Dimensions ScaleStopEngine(const int& aMaxHeight, const int& aMaxWidth)
{
    return {aMaxHeight - 2, static_cast<int>(aMaxWidth - 5 - cStopEngineMessage.length()), 0, 0};
}

XLinkWindow::XLinkWindow(WindowModel& aModel, std::string_view aTitle, std::function<Dimensions()> aCalculation) :
    Window(aModel, aTitle, std::move(aCalculation))
{
    SetUp();
}

void XLinkWindow::SetUp()
{
    Window::SetUp();
    // Get size of window so scaling works properly.
    GetSize();

    AddObject(std::make_unique<CheckBox>(
        *this,
        "Automatically search for XLink Kai instances",
        [&] { return ScaleSearchXLinkCheckBox(GetHeightReference(), GetWidthReference()); },
        GetModel().mAutoDiscoverXLinkKaiInstance));

    AddObject(std::make_unique<Button>(
        *this,
        cSaveMessage,
        [&] { return ScaleSaveButton(GetHeightReference(), GetWidthReference()); },
        [&] {
            GetModel().mCommand = WindowModel_Constants::Command::SaveSettings;
            return true;
        }));

    AddObject(std::make_unique<Button>(
        *this,
        cStartEngineMessage,
        [&] { return ScaleStartEngine(GetHeightReference(), GetWidthReference()); },
        [&] {
            GetModel().mCommand = WindowModel_Constants::Command::StartEngine;
            return true;
        }));

    AddObject(std::make_unique<Button>(
        *this,
        cStopEngineMessage,
        [&] { return ScaleStopEngine(GetHeightReference(), GetWidthReference()); },
        [&] {
            GetModel().mCommand = WindowModel_Constants::Command::StopEngine;
            return true;
        },
        false,
        false,
        false));

    AddObject(std::make_unique<String>(*this, "Press Tab to switch panes", [&] {
        return ScaleTabPressString(GetHeightReference(), GetWidthReference());
    }));
    AddObject(std::make_unique<String>(
        *this, cQuitMessage, [&] { return ScaleQQuitString(GetHeightReference(), GetWidthReference()); }));
}

void XLinkWindow::Draw()
{
    Window::Draw();
}