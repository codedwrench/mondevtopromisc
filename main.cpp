#include <iostream>
#include <memory>
#include <string>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <ncurses.h>
#undef timeout

#include "Includes/Logger.h"
#include "Includes/WirelessMonitorDevice.h"
#include "Includes/XLinkKaiConnection.h"

constexpr unsigned int cKeyQ{113};
constexpr unsigned int cKeyTab{9};

std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mMainWindow{nullptr};
std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mNetworkingWindow{nullptr};
std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mXLinkWindow{nullptr};
bool                                                  gRunning{true};
int                                                   mScreen{0};
int                                                   mLastKeyPressed{0};
int                                                   mWindowWidth{0};
int                                                   mWindowHeight{0};
bool                                                  mDimensionsChanged{false};

std::vector<std::vector<std::pair<bool, std::string>>> mCheckboxes{
    {{false, std::string("PSP/Vita Autoscan")},
     {false, std::string("Automatically try to put adapter in monitor mode")}},
     {{{false, std::string("Scan for XLink Kai instances automatically")}}}};

std::vector<unsigned int> mWindowSelections{0,0};
unsigned int mWindowSelector{0};
bool mFirstTime{true};

int  ProcessNetworkPanel();
int  ProcessXLinkPanel();
void ClearLine(WINDOW& aWindow, int aYCoord, int aLength);
void AddCheckBoxes(WINDOW& aWindow, int aStartingYCoord, std::vector<std::pair<bool, std::string>> aCheckBoxes);

int Process()
{
    refresh();
    if (!mFirstTime) {
        int mLastKeyPressed = getch();
        switch (mLastKeyPressed) {
            case cKeyQ:
                gRunning = false;
                break;
            case cKeyTab:
                mWindowSelector++;
                if (mWindowSelector == mWindowSelections.size()) {
                    mWindowSelector = 0;
                }
                break;
            case KEY_UP:
                if (mWindowSelections.at(mWindowSelector) != 0) {
                    mWindowSelections.at(mWindowSelector)--;
                }
                break;
            case KEY_DOWN:
                if (mWindowSelections.at(mWindowSelector) < mCheckboxes.at(mWindowSelector).size() - 1) {
                    mWindowSelections.at(mWindowSelector)++;
                }
                break;
            default:
                mLastKeyPressed = 0;
        }
    }

    // Get window size
    int lHeight{0};
    int lWidth{0};
    getmaxyx(mMainWindow.get(), lHeight, lWidth);
    if ((mWindowWidth != lWidth) || (mWindowHeight != lHeight)) {
        mWindowWidth       = lWidth;
        mWindowHeight      = lHeight;
        mDimensionsChanged = true;
    }

    switch (mScreen) {
        default:
            ProcessNetworkPanel();
            ProcessXLinkPanel();
    }

    std::string lStringToDraw{"Press TAB to switch panes"};
    mvwaddstr(mXLinkWindow.get(), (mWindowHeight / 2), 1, lStringToDraw.c_str());
    lStringToDraw = "Press q to quit";
    mvwaddstr(mXLinkWindow.get(), (mWindowHeight / 2), mWindowWidth - lStringToDraw.length() - 1, lStringToDraw.c_str());

    wrefresh(mMainWindow.get());
    wrefresh(mNetworkingWindow.get());
    wrefresh(mXLinkWindow.get());

    mFirstTime = false;

    return 1;
}

void ProcessCheckBoxes(WINDOW&                                    aWindow,
                       unsigned int                               aStartingYCoord,
                       unsigned int                               xCoord,
                       std::vector<std::pair<bool, std::string>>& aCheckBoxes,
                       int                                        aSelection)
{
    unsigned int lSelectionIndex{0};
    unsigned int lIndex{aStartingYCoord};
    for (const auto& lCheckBox : aCheckBoxes) {
        if (lSelectionIndex == aSelection) {
            wattrset(&aWindow, COLOR_PAIR(7));
        } else {
            wattrset(&aWindow, COLOR_PAIR(1));
        }
        std::string lStringToDraw{std::string("[") + (lCheckBox.first ? "X" : " ") + "]  " + lCheckBox.second};
        mvwaddstr(&aWindow, lIndex, xCoord, lStringToDraw.c_str());
        wattrset(&aWindow, COLOR_PAIR(1));
        lIndex++;
        lSelectionIndex++;
    }
}

int ProcessNetworkPanel()
{
    int lCheckboxesSelection{-1};

    if (mDimensionsChanged) {
        wresize(mNetworkingWindow.get(), mWindowHeight / 2, mWindowWidth);
    }

    // Clear background
    wattrset(mNetworkingWindow.get(), COLOR_PAIR(1));
    for (int lLine = 0; lLine <= mWindowHeight/2; lLine++) {
        ClearLine(*mNetworkingWindow, lLine, mWindowWidth);
    }

    if ((mWindowSelector == 0) && (mWindowSelections.at(0) < mCheckboxes.at(0).size())) {
        lCheckboxesSelection = mWindowSelections.at(0);
    }

    // Draw checkboxes
    ProcessCheckBoxes(*mNetworkingWindow, 2, 2, mCheckboxes.at(0), lCheckboxesSelection);

    // Draw header
    ClearLine(*mNetworkingWindow, 0, mWindowWidth);
    box(mNetworkingWindow.get(), 0, 0);
    wattrset(mNetworkingWindow.get(), COLOR_PAIR(7));
    std::string lHeaderText{"Network adapter options:"};
    mvwaddstr(mNetworkingWindow.get(), 0, 0, lHeaderText.c_str());
    wattrset(mNetworkingWindow.get(), COLOR_PAIR(1));

    curs_set(0);

    return 1;
}

int ProcessXLinkPanel()
{
    int lCheckboxesSelection{-1};

    if (mDimensionsChanged) {
        wresize(mXLinkWindow.get(), mWindowHeight / 2, mWindowWidth);
        mvwin(mXLinkWindow.get(), mWindowHeight / 2, mWindowWidth);
    }

    // Clear background
    wattrset(mXLinkWindow.get(), COLOR_PAIR(1));
    for (int lLine = 0; lLine <= mWindowHeight / 2; lLine++) {
        ClearLine(*mXLinkWindow, lLine, mWindowWidth);
    }

    if ((mWindowSelector == 1) && (mWindowSelections.at(1) < mCheckboxes.at(1).size())) {
        lCheckboxesSelection = mWindowSelections.at(1);
    }

    // Draw checkboxes
    ProcessCheckBoxes(*mXLinkWindow, 2, 2, mCheckboxes.at(1), lCheckboxesSelection);

    // Draw header
    ClearLine(*mXLinkWindow, 0, mWindowWidth);
    box(mXLinkWindow.get(), 0, 0);
    wattrset(mXLinkWindow.get(), COLOR_PAIR(7));
    std::string lHeaderText{"XLink Kai options:"};
    mvwaddstr(mXLinkWindow.get(), 0, 0, lHeaderText.c_str());
    wattrset(mXLinkWindow.get(), COLOR_PAIR(1));

    curs_set(0);

    return 1;
}

void ClearLine(WINDOW& aWindow, int aYCoord, int aLength)
{
    move(aYCoord, 1);
    aLength++;
    std::string lEmptySpace;
    lEmptySpace.resize(aLength, ' ');
    mvwaddstr(&aWindow, aYCoord, 0, lEmptySpace.c_str());
}

static void SignalHandler(const boost::system::error_code& aError, int aSignalNumber)
{
    if (!aError) {
        if (aSignalNumber == SIGINT || aSignalNumber == SIGTERM) {
            // Quit gracefully.
            gRunning = false;
        }
    }
}

int main(int argc, char* argv[])
{
    // Handle quit signals gracefully.
    boost::asio::io_service lSignalIoService{};
    boost::asio::signal_set lSignals(lSignalIoService, SIGINT, SIGTERM);
    lSignals.async_wait(&SignalHandler);
    boost::thread lThread{[lIoService = &lSignalIoService] { lIoService->run(); }};

    initscr();
    keypad(stdscr, true);
    nonl();
    cbreak();
    noecho();

    mMainWindow =
        std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(0, 0, 0, 0), [](WINDOW* aWin) { delwin(aWin); });

    getmaxyx(mMainWindow.get(), mWindowHeight, mWindowWidth);

    mNetworkingWindow = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(mWindowHeight / 2, 0, 0, 0),
                                                                              [](WINDOW* aWin) { delwin(aWin); });

    mXLinkWindow = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(0, 0, mWindowHeight / 2, 0),
                                                                         [](WINDOW* aWin) { delwin(aWin); });

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

    while (gRunning) {
        Process();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    endwin();

    mMainWindow = nullptr;

    lSignalIoService.stop();
    if (lThread.joinable()) {
        lThread.join();
    }
}
