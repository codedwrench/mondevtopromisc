#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - String.h
 *
 * This file contains an class for a userinterface string.
 *
 **/

#include "UIObject.h"
#include "IWindow.h"

class String : public UIObject
{
public:
    using UIObject::UIObject;
    void Draw() override;
};