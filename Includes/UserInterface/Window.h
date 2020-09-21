#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Window.h
 *
 * This file contains an class for a userinterface window.
 *
 **/

#include <functional>
#include <memory>

#include <ncurses.h>
#undef timeout

#include "IWindow.h"
#include "UIObject.h"

/**
 * Abstract class with basic functions to setup a window.
 */
class Window : public IWindow
{
public:
    Window(std::string_view aTitle,
           int              aYCoord,
           int              aXCoord,
           int              aLines,
           int              aColumns,
           bool             aExclusive = false,
           bool             aVisible   = true);

    Window(const Window& aWindow) = delete;
    Window& operator=(const Window& aWindow) = delete;

    Window& operator=(Window&& aWindow) = delete;
    Window(Window&& aWindow)            = default;

    ~Window() = default;

    void Draw() override;

    void AddObject(std::unique_ptr<IUIObject> aObject) override;

    void Refresh() override;

    bool Move(int aYCoord, int aXcoord) override;

    bool Resize(int aLines, int aColumns) override;

    bool AdvanceSelection() override;

    bool RecedeSelection() override;

    bool DoSelection() override;

    void SetExclusive(bool aExclusive) override;

    bool IsExclusive() override;

    void SetVisible(bool aVisible) override;

    bool IsVisible() override;

private:
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mNCursesWindow;
    std::string                                           mTitle;
    bool                                                  mExclusive;
    bool                                                  mVisible;
    std::vector<std::unique_ptr<IUIObject>>               mObjects;
};
