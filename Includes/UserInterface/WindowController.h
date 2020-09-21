#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - MainWindow.h
 *
 * This file contains an class for a userinterface main window.
 *
 **/

#include <ncurses.h>

#include "IWindow.h"
#undef timeout

#include <functional>
#include <memory>
#include <vector>

#include "Window.h"

/**
 * This class will setup our window layout and basically handle everything around the user interface.
 */
class WindowController
{
public:
    WindowController() = default;
    ~WindowController();

    WindowController(const WindowController& aWindowController) = delete;
    WindowController& operator=(const WindowController& aWindowController) = delete;

    WindowController& operator=(WindowController&& aWindowController) = delete;
    WindowController(WindowController&& aWindowController)            = delete;

    /**
     * Sets up the layout and assigns the keybinds and such.
     *
     * @return true if successful
     */
    bool SetUp();

    /**
     * Redraws and handles windows.
     */
    void Process();

private:
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mMainCanvas{nullptr};

    std::vector<std::unique_ptr<IWindow>> mWindows{};
    bool                                  mHasExclusiveAndVisibleWindow{false};
    int                                   mExclusiveWindowIndex{0};
};
