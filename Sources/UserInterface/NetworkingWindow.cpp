#include "../../Includes/UserInterface/NetworkingWindow.h"

#include <cmath>
#include <utility>

#include "../../Includes/UserInterface/CheckBox.h"
#include "../../Includes/UserInterface/TextField.h"

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingWindow.cpp */

Dimensions ScaleUsePSPPlugin(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {2, 2, 0, 0};
}

Dimensions ScaleUseWifiAdapter(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {3, 2, 0, 0};
}

Dimensions ScaleSetChannel(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {4, 2, 0, 0};
}

Dimensions ScaleSearchPSPNetworks(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {5, 2, 0, 0};
}

Dimensions ScaleAcknowledgeDataFrames(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {6, 2, 0, 0};
}

Dimensions ScaleOnlyAcceptFromMac(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {7, 2, 0, 0};
}


Dimensions ScaleSearchTakeHintsXLinkKai(const int& /*aMaxHeight*/, const int& /*aMaxWidth*/)
{
    return {8, 2, 0, 0};
}

NetworkingWindow::NetworkingWindow(WindowModel&                       aModel,
                                   std::string_view                   aTitle,
                                   const std::function<Dimensions()>& aCalculation) :
    Window(aModel, aTitle, aCalculation)
{
    SetUp();
}

void NetworkingWindow::SetUp()
{
    Window::SetUp();

    AddObject(std::make_shared<CheckBox>(
        *this,
        cUsePSPPlugin,
        [&] { return ScaleUsePSPPlugin(GetHeightReference(), GetWidthReference()); },
        GetModel().mUsePSPPlugin));

    AddObject(std::make_shared<TextField>(
        *this,
        cWifiAdapterToUse,
        [&] { return ScaleUseWifiAdapter(GetHeightReference(), GetWidthReference()); },
        GetModel().mWifiAdapter,
        15,
        true,
        true,
        std::vector<char>{}));

    AddObject(std::make_shared<TextField>(
        *this,
        cChannel,
        [&] { return ScaleSetChannel(GetHeightReference(), GetWidthReference()); },
        GetModel().mChannel,
        2,
        true,
        true,
        std::vector<char>{}));

    AddObject(std::make_shared<CheckBox>(
        *this,
        cScanWifiNetworksPSP,
        [&] { return ScaleSearchPSPNetworks(GetHeightReference(), GetWidthReference()); },
        GetModel().mAutoDiscoverPSPVitaNetworks));

    AddObject(std::make_shared<CheckBox>(
        *this,
        cEnableAcknowledgeDataFrames,
        [&] { return ScaleAcknowledgeDataFrames(GetHeightReference(), GetWidthReference()); },
        GetModel().mAcknowledgeDataFrames));


    AddObject(std::make_shared<TextField>(
        *this,
        cOnlyAcceptMac,
        [&] { return ScaleOnlyAcceptFromMac(GetHeightReference(), GetWidthReference()); },
        GetModel().mOnlyAcceptFromMac,
        17,
        true,
        true,
        std::vector<char>{':'}));

    // TODO: Add when XLink Kai adds it
    //    AddObject(std::make_shared<CheckBox>(
    //        *this,
    //        cTakeHintsFromXLinkKai,
    //        [&] { return ScaleSearchTakeHintsXLinkKai(GetHeightReference(), GetWidthReference()); },
    //        GetModel().mXLinkKaiHints));
}

void NetworkingWindow::Draw()
{
    // Do not show these if we are using a plugin
    std::shared_ptr<CheckBox> lPSPPluginCheckBox{std::dynamic_pointer_cast<CheckBox>(GetObjects().at(0))};
    GetObjects().at(2)->SetVisible(!lPSPPluginCheckBox->IsChecked());
    GetObjects().at(3)->SetVisible(!lPSPPluginCheckBox->IsChecked());
    GetObjects().at(4)->SetVisible(!lPSPPluginCheckBox->IsChecked());
    GetObjects().at(5)->SetVisible(!lPSPPluginCheckBox->IsChecked());

    Window::Draw();
}