#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include <boost/program_options.hpp>

#include <curses.h>

#include "Includes/IPCapDevice.h"
#undef timeout

#include "Includes/Logger.h"
#include "Includes/MonitorDevice.h"
#include "Includes/NetConversionFunctions.h"
#include "Includes/UserInterface/WindowController.h"
#include "Includes/WirelessPSPPluginDevice.h"
#include "Includes/XLinkKaiConnection.h"
namespace
{
    constexpr std::string_view cLogFileName{"log.txt"};
    constexpr std::string_view cPSPSSIDFilterName{"PSP_"};
    constexpr std::string_view cVitaSSIDFilterName{"SCE_"};
    constexpr bool             cLogToDisk{true};
    constexpr std::string_view cConfigFileName{"config.txt"};

    // Indicates if the program should be running or not, used to gracefully exit the program.
    bool gRunning{true};
}  // namespace

// Add npcap to the dll path for windows
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
bool InitNPcapDLLPath()
{
    bool        lReturn{false};
    std::string lNPcapDirectory{};
    lNPcapDirectory.resize(MAX_PATH);
    unsigned int lLength = GetSystemDirectory(lNPcapDirectory.data(), MAX_PATH);
    lNPcapDirectory.resize(lLength);
    if (lLength > 0) {
        lNPcapDirectory.append("\\Npcap");
        if (SetDllDirectory(lNPcapDirectory.data()) != 0) {
            lReturn = true;
        }
    }
    return lReturn;
}
#endif


static void SignalHandler(const boost::system::error_code& aError, int aSignalNumber)
{
    if (!aError) {
        if (aSignalNumber == SIGINT || aSignalNumber == SIGTERM) {
            // Quit gracefully.
            gRunning = false;
        }
    }
}

int main(int /*argc*/, char* argv[])
{
    std::string lProgramPath{"./"};

#if not defined(_WIN32) && not defined(_WIN64)
    // Make robust against sudo path change.
    std::array<char, PATH_MAX> lResolvedPath{};
    if (realpath(argv[0], lResolvedPath.data()) != nullptr) {
        lProgramPath = std::string(lResolvedPath.begin(), lResolvedPath.end());

        // Remove excecutable name from path
        size_t lExcecutableNameIndex{lProgramPath.rfind('/')};
        if (lExcecutableNameIndex != std::string::npos) {
            lProgramPath.erase(lExcecutableNameIndex + 1, lProgramPath.length() - lExcecutableNameIndex - 1);
        }
    }
#else
    // Npcap needs this
    if (!InitNPcapDLLPath()) {
        // Quit the application almost immediately
        gRunning = false;
    }
#endif

    // Handle quit signals gracefully.
    boost::asio::io_service lSignalIoService{};
    boost::asio::signal_set lSignals(lSignalIoService, SIGINT, SIGTERM);
    lSignals.async_wait(&SignalHandler);
    std::thread lThread{[lIoService = &lSignalIoService] { lIoService->run(); }};
    WindowModel mWindowModel{};
    mWindowModel.LoadFromFile(lProgramPath + cConfigFileName.data());

    Logger::GetInstance().Init(mWindowModel.mLogLevel, cLogToDisk, lProgramPath + cLogFileName.data());

    std::vector<std::string> lSSIDFilters{};
    WindowController         lWindowController(mWindowModel);
    lWindowController.SetUp();

    std::shared_ptr<IPCapDevice>        lDevice{nullptr};
    std::shared_ptr<XLinkKaiConnection> lXLinkKaiConnection{std::make_shared<XLinkKaiConnection>()};

    bool lSuccess{false};

    // If we need more entry methods, make an actual state machine
    bool                                               lWaitEntry{true};
    std::chrono::time_point<std::chrono::system_clock> lWaitStart{std::chrono::seconds{0}};

    while (gRunning) {
        if (lWindowController.Process()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            switch (mWindowModel.mCommand) {
                case WindowModel_Constants::Command::StartEngine:
                    if (mWindowModel.mLogLevel != Logger::GetInstance().GetLogLevel()) {
                        Logger::GetInstance().SetLogLevel(mWindowModel.mLogLevel);
                    }

                    // If we are using a PSP plugin device set up normal WiFi adapter
                    if (mWindowModel.mUsePSPPlugin) {
                        if (std::dynamic_pointer_cast<WirelessPSPPluginDevice>(lDevice) == nullptr) {
                            lDevice = std::make_shared<WirelessPSPPluginDevice>();
                        }
                    } else {
                        if (std::dynamic_pointer_cast<MonitorDevice>(lDevice) == nullptr) {
                            lDevice = std::make_shared<MonitorDevice>();
                            std::shared_ptr<MonitorDevice> lMonitorDevice =
                                std::dynamic_pointer_cast<MonitorDevice>(lDevice);
                            lMonitorDevice->SetSourceMACToFilter(MacToInt(mWindowModel.mOnlyAcceptFromMac));
                            lMonitorDevice->SetAcknowledgePackets(mWindowModel.mAcknowledgeDataFrames);
                        }
                    }
                    lXLinkKaiConnection->SetIncomingConnection(lDevice);
                    lDevice->SetConnector(lXLinkKaiConnection);

                    // If we are auto discovering PSP/VITA networks add those to the filter list
                    if (mWindowModel.mAutoDiscoverPSPVitaNetworks) {
                        lSSIDFilters.emplace_back(cPSPSSIDFilterName.data());
                        lSSIDFilters.emplace_back(cVitaSSIDFilterName.data());
                    }

                    // Set the XLink Kai connection up, if we are autodiscovering we don't need to provide an IP
                    if (!mWindowModel.mAutoDiscoverXLinkKaiInstance) {
                        lSuccess = lXLinkKaiConnection->Open(mWindowModel.mXLinkIp, std::stoi(mWindowModel.mXLinkPort));
                    } else {
                        lSuccess = lXLinkKaiConnection->Open("");
                    }

                    // Now set up the wifi interface
                    if (lSuccess) {
                        if (lDevice->Open(mWindowModel.mWifiAdapter, lSSIDFilters)) {
                            if (lDevice->StartReceiverThread() && lXLinkKaiConnection->StartReceiverThread()) {
                                mWindowModel.mEngineStatus = WindowModel_Constants::EngineStatus::Running;
                                mWindowModel.mCommand      = WindowModel_Constants::Command::NoCommand;
                            } else {
                                Logger::GetInstance().Log("Failed to start receiver threads", Logger::Level::ERROR);
                                mWindowModel.mEngineStatus     = WindowModel_Constants::EngineStatus::Error;
                                mWindowModel.mCommand          = WindowModel_Constants::Command::WaitForTime;
                                mWindowModel.mTimeToWait       = std::chrono::seconds(5);
                                mWindowModel.mCommandAfterWait = WindowModel_Constants::Command::StopEngine;
                            }
                        } else {
                            Logger::GetInstance().Log("Failed to activate monitor interface", Logger::Level::ERROR);
                            mWindowModel.mEngineStatus     = WindowModel_Constants::EngineStatus::Error;
                            mWindowModel.mCommand          = WindowModel_Constants::Command::WaitForTime;
                            mWindowModel.mTimeToWait       = std::chrono::seconds(5);
                            mWindowModel.mCommandAfterWait = WindowModel_Constants::Command::StopEngine;
                        }
                    } else {
                        Logger::GetInstance().Log("Failed to open connection to XLink Kai, retrying in 10 seconds!",
                                                  Logger::Level::ERROR);
                        // Have it take some time between tries
                        mWindowModel.mCommand          = WindowModel_Constants::Command::WaitForTime;
                        mWindowModel.mTimeToWait       = std::chrono::seconds(10);
                        mWindowModel.mCommandAfterWait = WindowModel_Constants::Command::NoCommand;
                    }
                    break;
                case WindowModel_Constants::Command::WaitForTime:
                    // Wait state, use this to add a delay without making the UI unresponsive.
                    if (lWaitEntry) {
                        lWaitStart = std::chrono::system_clock::now();
                        lWaitEntry = false;
                    }

                    if (std::chrono::system_clock::now() > lWaitStart + mWindowModel.mTimeToWait) {
                        mWindowModel.mCommand = mWindowModel.mCommandAfterWait;
                        lWaitEntry            = true;
                    }
                    break;
                case WindowModel_Constants::Command::StopEngine:
                    lXLinkKaiConnection->Close();
                    lDevice->Close();
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
                    mWindowModel.SaveToFile(cConfigFileName);
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
    exit(0);
}
