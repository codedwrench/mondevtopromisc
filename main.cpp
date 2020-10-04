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
#include "Includes/PCapReader.h"

namespace
{
    constexpr Logger::Level cLogLevel{Logger::Level::TRACE};
    constexpr char          cLogFileName[]{"log.txt"};
    constexpr bool          cLogToDisk{true};

    // Indicates if the program should be running or not, used to gracefully exit the program.
    bool gRunning{true};
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
    Logger::GetInstance().Init(cLogLevel, cLogToDisk, cLogFileName);
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
            switch(mWindowModel.mCommand) {
                case WindowModel_Constants::Command::StartEngine:
                    mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Running;
                    mWindowModel.mCommand      = WindowModel_Constants::Command::NoCommand;
                    break;
                case WindowModel_Constants::Command::StopEngine:
                    mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Idle;
                    mWindowModel.mCommand = WindowModel_Constants::Command::NoCommand;
                    break;
                case WindowModel_Constants::Command::StartSearchNetworks:
                    // TODO: implement.
                    break;
                case WindowModel_Constants::Command::StopSearchNetworks:
                    // TODO: implement.
                    break;
                case WindowModel_Constants::Command::SaveSettings:
                    // TODO: implement.
                    break;
                case WindowModel_Constants::Command::NoCommand:
                    break;
            }
        } else {
            gRunning = false;
        }
    }

    lSignalIoService.stop();
    if (lThread.joinable()) {
        lThread.join();
    }
}
