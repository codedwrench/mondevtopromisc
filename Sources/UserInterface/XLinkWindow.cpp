#include "../../Includes/UserInterface/XLinkWindow.h"

#include <cmath>
#include <utility>

#include "../../Includes/UserInterface/Button.h"
#include "../../Includes/UserInterface/CheckBox.h"
#include "../../Includes/UserInterface/String.h"
#include "../../Includes/UserInterface/TextField.h"

/* Copyright (c) 2020 [Rick de Bondt] - XLinkWindow.cpp */

Dimensions ScaleSearchXLinkCheckBox(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {2, 2, 0, 0};
}

Dimensions ScaleIpAddressTextField(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {3, 2, 0, 0};
}

Dimensions ScalePortTextField(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {4, 2, 0, 0};
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

Dimensions XLinkWindow::ScaleStatusMessage(const int& aMaxHeight, const int& aMaxWidth)
{
    // This long calculation really just calculates to half width of the screen.
    return {aMaxHeight - 1,
            static_cast<int>(
                floor(aMaxWidth / 2.0) -
                static_cast<int>(floor(static_cast<int>(cStatusPrefix.length() + 1 +
                                                        WindowModel_Constants::cEngineStatusTexts
                                                            .at(static_cast<unsigned long>(GetModel().mEngineStatus))
                                                            .length()) /
                                       2.0))),
            0,
            0};
}

XLinkWindow::XLinkWindow(WindowModel&                       aModel,
                         std::string_view                   aTitle,
                         const std::function<Dimensions()>& aCalculation) :
    Window(aModel, aTitle, aCalculation)
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
        cAutoSearchXLinkInstancesMessage,
        [&] { return ScaleSearchXLinkCheckBox(GetHeightReference(), GetWidthReference()); },
        GetModel().mAutoDiscoverXLinkKaiInstance));

    AddObject(std::make_unique<TextField>(
            *this,
            cIPAddressMessage,
            [&] { return ScaleIpAddressTextField(GetHeightReference(), GetWidthReference()); },
            GetModel().mXLinkIp,
            15));

    AddObject(std::make_unique<TextField>(
            *this,
            cPortMessage,
            [&] { return ScalePortTextField(GetHeightReference(), GetWidthReference()); },
            GetModel().mXLinkPort,
            5));

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

    AddObject(std::make_unique<String>(
        *this, cTabMessage, [&] { return ScaleTabPressString(GetHeightReference(), GetWidthReference()); }));

    AddObject(std::make_unique<String>(
        *this,
        std::string(cStatusPrefix) + cDefaultStatusMessage.data(),
        [&] { return ScaleStatusMessage(GetHeightReference(), GetWidthReference()); },
        false,
        false,
        7));

    AddObject(std::make_unique<String>(
        *this, cQuitMessage, [&] { return ScaleQQuitString(GetHeightReference(), GetWidthReference()); }));
}

void XLinkWindow::Draw()
{
    // TODO: find better way to set status.
    GetObjects().at(7)->SetName(
        std::string(cStatusPrefix) +
        WindowModel_Constants::cEngineStatusTexts.at(static_cast<unsigned long>(GetModel().mEngineStatus)) + " ");
    GetObjects().at(7)->Scale();

    Window::Draw();
}