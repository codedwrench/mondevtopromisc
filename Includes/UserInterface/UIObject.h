#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - UIObject.h
 *
 * This file contains an abstract class for a userinterface object.
 *
 **/

#include <string>
#include <string_view>

#include "IUIObject.h"
#include "Window.h"

class UIObject : public IUIObject
{
public:
    UIObject(IWindow&                    aWindow,
             std::string_view            aName,
             std::function<Dimensions()> aCalculation,
             bool                        aVisible    = true,
             bool                        aSelectable = false);

    bool DoAction() override;
    void Scale() override;

    void               SetVisible(bool aVisible) override;
    [[nodiscard]] bool IsVisible() const override;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelectable() const override;
    [[nodiscard]] bool IsSelected() const override;
    [[nodiscard]] int  GetYCoord() const override;
    [[nodiscard]] int  GetXCoord() const override;
    std::string_view   GetName() override;
    void               SetName(std::string_view) override;

protected:
    IWindow& GetWindow();

private:
    IWindow&                    mWindow;
    std::string                 mName;
    bool                        mSelectable;
    bool                        mVisible;
    int                         mYCoord;
    int                         mXCoord;
    std::function<Dimensions()> mScaleCalculation;
};