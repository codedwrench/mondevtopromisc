#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - CheckBox.h
 *
 * This file contains an class for a userinterface checkbox.
 *
 **/

#include "IWindow.h"
#include "UIObject.h"

/**
 * Class for a userinterface checkbox.
 */
class CheckBox : public UIObject
{
public:
    CheckBox(IWindow&                    aWindow,
             std::string_view            aName,
             std::function<Dimensions()> aCalculation,
             bool&                       aModelCheckBox,
             bool                        aSelected   = false,
             bool                        aChecked    = false,
             bool                        aVisible    = true,
             bool                        aSelectable = true);

    void Draw() override;
    bool DoAction() override;

    void               SetChecked(bool aChecked);
    [[nodiscard]] bool IsChecked() const;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelected() const override;

private:
    bool& mModelCheckBox;
    bool  mSelected;
    bool  mChecked;
};
