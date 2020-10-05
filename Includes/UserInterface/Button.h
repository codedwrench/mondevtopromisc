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
    Button(IWindow&                    aWindow,
           std::string_view            aName,
           std::function<Dimensions()> aCalculation,
           std::function<bool()>       aAction,
           bool                        aSelected   = false,
           bool                        aVisible    = true,
           bool                        aSelectable = true);

    void Draw() override;
    bool HandleKey(unsigned int aKeyCode) override;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelected() const override;

private:
    std::function<bool()> mAction;
    bool                  mSelected;
};
