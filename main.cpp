#include <iostream>
#include <memory>
#include <string>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <ncurses.h>
#undef timeout

#include "Includes/Logger.h"
#include "Includes/UserInterface/WindowController.h"
#include "Includes/WirelessMonitorDevice.h"
#include "Includes/XLinkKaiConnection.h"

bool gRunning{true};


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
    WindowModel   mWindowModel{};

    WindowController lWindowController(mWindowModel);
    lWindowController.SetUp();

    while (gRunning) {
        if (lWindowController.Process()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            gRunning = false;
        }
    }

    lSignalIoService.stop();
    if (lThread.joinable()) {
        lThread.join();
    }
}
