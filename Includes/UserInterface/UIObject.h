#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - UIObject.h
 *
 * This file contains an class for a userinterface object.
 *
 **/

#include "IUIObject.h"

/**
 * This class contains the bare minimum for a UI object to exist.
 */
class UIObject : public IUIObject
{
public:
    void Draw() override;
    bool DoAction() override;
};
