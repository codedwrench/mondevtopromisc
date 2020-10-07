#include <iostream>
#include <memory>
#include <string>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <curses.h>
#undef timeout

#include "Includes/Logger.h"
#include "Includes/UserInterface/WindowController.h"
#include "Includes/WirelessMonitorDevice.h"
#include "Includes/XLinkKaiConnection.h"

namespace
{
    constexpr Logger::Level    cLogLevel{Logger::Level::TRACE};
    constexpr std::string_view cLogFileName{"log.txt"};
    constexpr std::string_view cPSPSSIDFilterName{"PSP_"};
    constexpr std::string_view cVitaSSIDFilterName{"SCE_"};
    constexpr bool             cLogToDisk{true};

    // Indicates if the program should be running or not, used to gracefully exit the program.
    bool gRunning{true};
}  // namespace


static void SignalHandler(const boost::system::error_code& aError, int aSignalNumber)
{
    if (!aError) {
        if (aSignalNumber == SIGINT || aSignalNumber == SIGTERM) {
            // Quit gracefully.
            gRunning = false;
        }
    }
}

int ConvertChannelToFrequency(int aChannel)
{
    int lReturn{-1};

    // 2.4GHz, steps of 5hz.
    if (aChannel >= 1 && aChannel <= 13) {
        lReturn = 2412 + ((aChannel - 1) * 5);
    }

    return lReturn;
}

int main(int argc, char* argv[])
{
    Logger::GetInstance().Init(cLogLevel, cLogToDisk, cLogFileName.data());

    // Handle quit signals gracefully.
    boost::asio::io_service lSignalIoService{};
    boost::asio::signal_set lSignals(lSignalIoService, SIGINT, SIGTERM);
    lSignals.async_wait(&SignalHandler);
    boost::thread lThread{[lIoService = &lSignalIoService] { lIoService->run(); }};
    WindowModel   mWindowModel{};

    std::vector<std::string> lSSIDFilters{};
    WindowController         lWindowController(mWindowModel);
    lWindowController.SetUp();

    std::shared_ptr<WirelessMonitorDevice> lMonitorDevice{std::make_shared<WirelessMonitorDevice>()};
    std::shared_ptr<XLinkKaiConnection>    lXLinkKaiConnection{std::make_shared<XLinkKaiConnection>()};

    lMonitorDevice->SetSendReceiveDevice(lXLinkKaiConnection);
    lXLinkKaiConnection->SetSendReceiveDevice(lMonitorDevice);

    bool lSuccess{false};

    while (gRunning) {
        if (lWindowController.Process()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            switch (mWindowModel.mCommand) {
                case WindowModel_Constants::Command::StartEngine:

                    // If we are auto discovering PSP/VITA networks add those to the filter list
                    if (mWindowModel.mAutoDiscoverPSPVitaNetworks) {
                        lSSIDFilters.emplace_back(cPSPSSIDFilterName.data());
                        lSSIDFilters.emplace_back(cVitaSSIDFilterName.data());
                    }

                    // Set the XLink Kai connection up, if we are autodiscovering we don't need to provide an IP
                    if (!mWindowModel.mAutoDiscoverXLinkKaiInstance) {
                        lSuccess = lXLinkKaiConnection->Open(mWindowModel.mXLinkIp, std::stoi(mWindowModel.mXLinkPort));
                    } else {
                        lSuccess = lXLinkKaiConnection->Open();
                    }

                    // Now set up the wifi interface
                    if (lSuccess) {
                        if (lMonitorDevice->Open(mWindowModel.mWifiAdapter,
                                                 lSSIDFilters,
                                                 ConvertChannelToFrequency(std::stoi(mWindowModel.mChannel)))) {
                            
                            if (lMonitorDevice->StartReceiverThread() && lXLinkKaiConnection->StartReceiverThread())
                            {
                                mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Running;
                            }
                            else {
                                Logger::GetInstance().Log("Failed to start receiver threads", Logger::Level::ERROR);
                                mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Error;
                            }
                        } else {
                            Logger::GetInstance().Log("Failed to activate monitor interface", Logger::Level::ERROR);
                            mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Error;
                        }
                    } else {
                        Logger::GetInstance().Log("Failed to open connection to XLink Kai", Logger::Level::ERROR);
                        mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Error;
                    }

                    mWindowModel.mCommand = WindowModel_Constants::Command::NoCommand;
                    break;
                case WindowModel_Constants::Command::StopEngine:
                    lXLinkKaiConnection->Close();
                    lMonitorDevice->Close();
                    lSSIDFilters.clear();
                    
                    mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Idle;
                    mWindowModel.mCommand      = WindowModel_Constants::Command::NoCommand;
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
