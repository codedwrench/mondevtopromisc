#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - CheckBox.h
 *
 * This file contains an class for a userinterface checkbox.
 *
 **/

#include "IWindow.h"
#include "UIObject.h"

class CheckBox : public UIObject
{
public:
    CheckBox(IWindow&                                                         aWindow,
             std::string_view                                                 aName,
             const std::function<std::array<int, 4>(const int&, const int&)>& aScaleCalculation,
             const int&                                                       aMaxHeight,
             const int&                                                       aMaxWidth,
             bool&                                                            aModelCheckBox,
             bool                                                             aSelected   = false,
             bool                                                             aChecked    = false,
             bool                                                             aVisible    = true,
             bool                                                             aSelectable = true);

    void Draw() override;
    bool DoAction() override;

    void               SetChecked(bool aChecked);
    [[nodiscard]] bool IsChecked() const;

    void               SetSelected(bool aSelected);
    [[nodiscard]] bool IsSelected() const;

private:
    bool& mModelCheckBox;
    bool  mSelected;
    bool  mChecked;
};
