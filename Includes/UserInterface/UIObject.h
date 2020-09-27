#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - UIObject.h
 *
 * This file contains an abstract class for a userinterface object.
 *
 **/

#include <string>
#include <string_view>

#include "IUIObject.h"
#include "IWindow.h"

class UIObject : public IUIObject
{
public:
    UIObject(IWindow&         aWindow,
             std::string_view aName,
             int              aYCoord,
             int              aXCoord,
             bool             aVisible    = true,
             bool             aSelectable = false);

    void Move(int aYCoord, int aXCoord) override;
    bool DoAction() override;

    void               SetVisible(bool aVisible);
    [[nodiscard]] bool IsVisible() const;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelectable() const override;
    [[nodiscard]] bool IsSelected() const override;

protected:
    IWindow&         GetWindow();
    std::string_view GetName();
    int              GetYCoord() const;
    int              GetXCoord() const;

private:
    IWindow&    mWindow;
    std::string mName;
    bool        mSelectable;
    bool        mVisible;
    int         mYCoord;
    int         mXCoord;
};