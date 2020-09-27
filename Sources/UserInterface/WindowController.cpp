#include "../../Includes/UserInterface/WindowController.h"

/* Copyright (c) 2020 [Rick de Bondt] - WindowController.cpp */

#include <cmath>

#include "../../Includes/UserInterface/NetworkingWindow.h"
#include "../../Includes/UserInterface/XLinkWindow.h"

std::array<int, 4> NetworkingWindowScaleFunction(const int& aMaxHeight, const int& aMaxWidth)
{
    return {0, 0, static_cast<int>(floor(aMaxHeight / 2.0)), aMaxWidth};
}

std::array<int, 4> XLinkWindowScaleFunction(const int& aMaxHeight, const int& aMaxWidth)
{
    return {static_cast<int>(floor(aMaxHeight / 2.0)), 0, static_cast<int>(floor(aMaxHeight / 2.0)), aMaxWidth};
}

std::array<int, 4> SSIDSelectWindowScaleFunction(const int& aMaxHeight, const int& aMaxWidth)
{
    return {10, 10, aMaxHeight - 20, aMaxWidth - 20};
}

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

    mWindows.emplace_back(
        std::make_shared<NetworkingWindow>("Networking pane:", NetworkingWindowScaleFunction, mHeight, mWidth));

    mWindows.emplace_back(std::make_shared<XLinkWindow>("XLink Kai pane:", XLinkWindowScaleFunction, mHeight, mWidth));

    mWindows.emplace_back(
        std::make_shared<Window>("SSID Selection:", SSIDSelectWindowScaleFunction, mHeight, mWidth, false, false));

    return true;
}

void WindowController::Process()
{
    int lHeight{0};
    int lWidth{0};
    getmaxyx(mMainCanvas.get(), lHeight, lWidth);

    if ((lHeight != mHeight) || (lWidth != mWidth)) {
        mHeight            = lHeight;
        mWidth             = lWidth;
        mDimensionsChanged = true;
    }

    wrefresh(mMainCanvas.get());

    if (mExclusiveWindow != nullptr) {
        if (!mExclusiveWindow->IsVisible() || !(mExclusiveWindow->IsExclusive())) {
            mExclusiveWindow = nullptr;
        } else {
            if (mDimensionsChanged) {
                mExclusiveWindow->Scale();
            }
            mExclusiveWindow->Draw();
        }
    }

    if (mExclusiveWindow == nullptr) {
        for (auto& lWindow : mWindows) {
            if (lWindow->IsVisible()) {
                if (mDimensionsChanged) {
                    for (int lCount = 0; lCount < mHeight; lCount++) {
                        std::string lString{};
                        lString.resize(mWidth, ' ');
                        mvwaddstr(mMainCanvas.get(), lCount, 0, lString.c_str());
                    }
                    lWindow->Scale();
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