#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Window.h
 *
 * This file contains an class for a userinterface window.
 *
 **/

#include "IWindow.h"

/**
 * Abstract class with basic functions to setup a window.
 */
class Window : public IWindow
{
public:
    void Draw() override;

    void AddObject() override;

    void Refresh() override;

    bool Move(unsigned int aYCoord, unsigned int aXcoord) override;

    bool Resize(unsigned int aLines, unsigned int aColumns) override;

    bool AdvanceSelection() override;

    bool RecedeSelection() override;

    bool DoSelection() override;
};
