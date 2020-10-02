#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - String.h
 *
 * This file contains an class for a userinterface string.
 *
 **/

#include "IWindow.h"
#include "UIObject.h"

class String : public UIObject
{
public:
    String(IWindow&                    aWindow,
           std::string_view            aName,
           std::function<Dimensions()> aCalculation,
           bool                        aVisible    = true,
           bool                        aSelectable = false,
           int                         aColorPair  = 1);
    void Draw() override;
    void SetColorPair(int aPair);

private:
    int mColorPair{1};
};