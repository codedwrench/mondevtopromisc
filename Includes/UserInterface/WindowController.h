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
    /**
     * Constructor of WindowController, handles everything around the user interface.
     * @param aModel - A model where the window can leave its data for the rest of the program.
     */
    WindowController(WindowModel& aModel);
    ~WindowController();

    WindowController(const WindowController& aWindowController) = delete;
    WindowController& operator=(const WindowController& aWindowController) = delete;

    WindowController& operator=(WindowController&& aWindowController) = delete;
    WindowController(WindowController&& aWindowController)            = delete;

    /**
     * Sets up the layout and assigns the keybinds and such.
     * @return true if successful
     */
    bool SetUp();

    /**
     * Redraws and handles windows.
     * @return true if still processing
     */
    bool Process();

private:
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mMainCanvas;

    int                                      mHeight;
    int                                      mWidth;
    bool                                     mDimensionsChanged;
    std::vector<std::shared_ptr<IWindow>>    mWindows;
    bool                                     mExclusiveWindow;
    std::pair<int, std::shared_ptr<IWindow>> mWindowSelector;
    WindowModel&                             mModel;
};
