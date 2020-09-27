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
    using UIObject::UIObject;
    void Draw() override;
};