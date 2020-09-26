#include "../../Includes/UserInterface/WindowController.h"

/* Copyright (c) 2020 [Rick de Bondt] - WindowController.cpp */

#include <cmath>

#include "../../Includes/UserInterface/NetworkingWindow.h"
#include "../../Includes/UserInterface/XLinkWindow.h"

bool WindowController::SetUp()
{
    initscr();
    keypad(stdscr, true);
    nonl();
    cbreak();
    noecho();
    nodelay(stdscr, true);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_WHITE, COLOR_BLUE);
        init_pair(3, COLOR_BLACK, COLOR_CYAN);
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        init_pair(5, COLOR_CYAN, COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_BLACK, COLOR_WHITE);
    }

    mMainCanvas =
        std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(0, 0, 0, 0), [](WINDOW* aWin) { delwin(aWin); });

    getmaxyx(mMainCanvas.get(), mHeight, mWidth);
    mWindows.emplace_back(std::make_shared<NetworkingWindow>("Networking pane:", 0, 0, (floor(mHeight / 2.0)), mWidth));

    mWindows.emplace_back(
        std::make_shared<XLinkWindow>("XLink Kai pane:", floor(mHeight / 2.0), 0, floor(mHeight / 2.0), mWidth));

    mWindows.emplace_back(std::make_shared<Window>("SSID Selection:", 10, 10, mHeight - 10, mWidth - 10, false, false));

    return true;
}

void WindowController::Process()
{
    int lHeight{0};
    int lWidth{0};
    getmaxyx(mMainCanvas.get(), lHeight, lWidth);

    if((lHeight != mHeight) || (lWidth != mWidth))
    {
        mHeight = lHeight;
        mWidth = lWidth;
        mDimensionsChanged = true;
    }

    wrefresh(mMainCanvas.get());

    if (mExclusiveWindow != nullptr) {
        if (!mExclusiveWindow->IsVisible() || !(mExclusiveWindow->IsExclusive())) {
            mExclusiveWindow = nullptr;
        } else {
            if(mDimensionsChanged)
            {
                mExclusiveWindow->Scale(lHeight, lWidth);
            }
            mExclusiveWindow->Draw();
        }
    }

    if (mExclusiveWindow == nullptr) {
        for (auto& lWindow : mWindows) {
            if (lWindow->IsVisible()) {
                if(mDimensionsChanged)
                {
                    lWindow->Scale(lHeight, lWidth);
                }
                lWindow->Draw();
                if (lWindow->IsExclusive()) {
                    mExclusiveWindow = lWindow;
                }
            }
        }
    }

    mDimensionsChanged = false;
    curs_set(0);
}

WindowController::~WindowController()
{
    endwin();
}