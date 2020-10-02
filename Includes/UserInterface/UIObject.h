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
    UIObject(IWindow&         aWindow,
             std::string_view aName,
             ScaleCalculation aCalculation,
             const int&       aMaxHeight,
             const int&       aMaxWidth,
             bool             aVisible    = true,
             bool             aSelectable = false);

    bool DoAction() override;
    void Scale() override;

    void               SetVisible(bool aVisible);
    [[nodiscard]] bool IsVisible() const;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelectable() const override;
    [[nodiscard]] bool IsSelected() const override;
    [[nodiscard]] int  GetYCoord() const override;
    [[nodiscard]] int  GetXCoord() const override;
    std::string_view   GetName() override;

protected:
    IWindow& GetWindow();

private:
    IWindow&                                 mWindow;
    std::string                              mName;
    bool                                     mSelectable;
    bool                                     mVisible;
    int                                      mYCoord;
    int                                      mXCoord;
    const Window_Constants::ScaleCalculation mScaleCalculation;
    const int&                               mMaxHeight;
    const int&                               mMaxWidth;
};