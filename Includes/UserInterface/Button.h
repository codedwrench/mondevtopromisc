#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Button.h
 *
 * This file contains an class for a userinterface button.
 *
 **/

#include <functional>

#include "UIObject.h"

/**
 * Class for a userinterface button.
 */
class Button : public UIObject
{
public:
    Button(IWindow&              aWindow,
           std::string_view      aName,
           ScaleCalculation      aCalculation,
           const int&            aMaxHeight,
           const int&            aMaxWidth,
           std::function<bool()> aAction,
           bool                  aSelected   = false,
           bool                  aChecked    = false,
           bool                  aVisible    = true,
           bool                  aSelectable = true);

    void Draw() override;
    bool DoAction() override;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelected() const override;

private:
    std::function<bool()> mAction;
    bool                  mSelected;
    bool                  mChecked;
};
