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

std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mMainWindow{nullptr};
std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mNetworkingWindow{nullptr};
std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> mXLinkWindow{nullptr};
bool                                                  gRunning{true};
int                                                   mScreen{0};
int                                                   mLastKeyPressed{0};
int                                                   mWindowWidth{0};
int                                                   mWindowHeight{0};
bool                                                  mDimensionsChanged{false};

int  ProcessNetworkPanel();
int  ProcessXLinkPanel();
void ClearLine(WINDOW& aWindow, int aYCoord, int aLength);

int Process()
{
    // Get window size
    int lHeight{0};
    int lWidth{0};
    getmaxyx(mMainWindow.get(), lHeight, lWidth);
    if ((mWindowWidth != lWidth) || (mWindowHeight != lHeight))
    {
        mWindowWidth = lWidth;
        mWindowHeight = lHeight;
        mDimensionsChanged = true;
    }

    switch (mScreen) {
        default:
            ProcessNetworkPanel();
            ProcessXLinkPanel();
    }

    if (mLastKeyPressed == cKeyQ) {
        gRunning = false;
    }

    refresh();

    return 1;
}

int ProcessNetworkPanel()
{
    if (mDimensionsChanged)
    {
        wresize(mNetworkingWindow.get(), mWindowHeight/2, mWindowWidth);
    }

    // Clear background
    wattrset(mNetworkingWindow.get(), COLOR_PAIR(1));
    for (int lLine = 0; lLine <= mWindowHeight; lLine++) {
        ClearLine(*mNetworkingWindow, lLine, mWindowWidth);
    }

    // Draw header
    ClearLine(*mNetworkingWindow, 0, mWindowWidth);
    box(mNetworkingWindow.get(), 0, 0);
    wattrset(mNetworkingWindow.get(), COLOR_PAIR(7));
    std::string lHeaderText{"Network adapter options:"};
    mvwaddstr(mNetworkingWindow.get(), 0, 0, lHeaderText.c_str());

    curs_set(0);

    wrefresh(mNetworkingWindow.get());

    return 1;
}

int ProcessXLinkPanel()
{
    if (mDimensionsChanged)
    {
        mvwin(mXLinkWindow.get(), mWindowHeight/2, 0);
        wresize(mXLinkWindow.get(), mWindowHeight/2, mWindowWidth);
    }

    // Clear background
    wattrset(mXLinkWindow.get(), COLOR_PAIR(1));
    for (int lLine = 0; lLine <= mWindowHeight; lLine++) {
        ClearLine(*mXLinkWindow, lLine, mWindowWidth);
    }

    // Draw header
    ClearLine(*mXLinkWindow, 0, mWindowWidth);
    box(mXLinkWindow.get(), 0, 0);
    wattrset(mXLinkWindow.get(), COLOR_PAIR(7));
    std::string lHeaderText{"XLink Kai options:"};
    mvwaddstr(mXLinkWindow.get(), 0, 0, lHeaderText.c_str());

    curs_set(0);

    wrefresh(mXLinkWindow.get());

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
<<<<<<< HEAD
    std::string             lBssid{};
    std::string             lCaptureInterface{};
    std::string             lInjectionInterface{};
    po::options_description lDescription("Options");
    // clang-format off
    lDescription.add_options()
            ("help,h", "Shows this help message.")
            ("bssid,b", po::value<std::string>(&lBssid)->required(), "The BSSID to listen to.")
            ("capture_interface,c", po::value<std::string>(&lCaptureInterface)->required(),
             "The interface that will be in monitor mode listening to packets.")
            ("injection_interface,i", po::value<std::string>(&lInjectionInterface),
             "If your interface does not support packet injection, another interface can be used")
            ("xlink_kai,x", "Enables the XLink Kai interface, so packets will be sent there directly");
    // clang-format on
    po::variables_map lVariableMap;
    po::store(po::command_line_parser(argc, argv).options(lDescription).run(), lVariableMap);

    if (lVariableMap.count("help") || lVariableMap.count("h")) {
        std::cout << lDescription << std::endl;
    } else {
        po::notify(lVariableMap);

        // Handle quit signals gracefully.
        boost::asio::io_service lSignalIoService{};
        boost::asio::signal_set lSignals(lSignalIoService, SIGINT, SIGTERM);
        lSignals.async_wait(&SignalHandler);
        boost::thread lThread{[lIoService = &lSignalIoService] { lIoService->run(); }};


        Logger::GetInstance().Init(cLogLevel, cLogToDisk, cLogFileName);
        std::shared_ptr<WirelessMonitorDevice> lMonitorDevice = std::make_shared<WirelessMonitorDevice>();
        std::shared_ptr<ISendReceiveDevice>    lOutputDevice  = nullptr;

        if (lVariableMap.count("xlink_kai")) {
            std::shared_ptr<XLinkKaiConnection> lXLinkKai = std::make_shared<XLinkKaiConnection>();
            if (lXLinkKai->Open(XLinkKai_Constants::cIp)) {
                lXLinkKai->SetSendReceiveDevice(lMonitorDevice);
                if (lXLinkKai->StartReceiverThread()) {
                    lOutputDevice = lXLinkKai;
                } else {
                    lXLinkKai->Close();
                }
            } else {
                Logger::GetInstance().Log("Opening of XLink Kai device failed!", Logger::Level::ERROR);
            }
        }
=======
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
>>>>>>> 6a3c640... Added basic TUI code.

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

