#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - CheckBox.h
 *
 * This file contains an class for a userinterface checkbox.
 *
 **/

#include "UIObject.h"
#include "IWindow.h"

class CheckBox : public UIObject
{
public:
    CheckBox(IWindow& aWindow, std::string_view aName, int aYCoord, int aXCoord, bool aChecked = false, bool aSelected = false, bool aVisible = true);
    void Draw() override;
    bool DoAction() override;

    void               SetChecked(bool aChecked);
    [[nodiscard]] bool IsChecked() const;

    void               SetSelected(bool aSelected);
    [[nodiscard]] bool IsSelected() const;

private:
    bool mSelected;
    bool mChecked;
};
