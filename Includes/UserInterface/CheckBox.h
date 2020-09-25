#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - CheckBox.h
 *
 * This file contains an class for a userinterface checkbox.
 *
 **/

#include "IUIObject.h"
#include "IWindow.h"

class CheckBox : public IUIObject
{
public:
    CheckBox(IWindow& aWindow, std::string_view aName, int aYCoord, int aXCoord, bool aChecked, bool aSelected);
    void Draw() override;
    bool DoAction() override;

    void               SetChecked(bool aChecked);
    [[nodiscard]] bool IsChecked() const;

    void               SetSelected(bool aSelected);
    [[nodiscard]] bool IsSelected() const;

private:
    IWindow&    mWindow;
    std::string mName;
    bool        mChecked;
    bool        mSelected;
    int         mYCoord;
    int         mXCoord;
};