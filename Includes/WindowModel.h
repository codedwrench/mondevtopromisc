#pragma once

#include <array>
#include <string>

#include "../Includes/Logger.h"

namespace WindowModel_Constants
{
    const std::string_view cSaveFilePath{"config.txt"};

    const std::string_view cSaveLogLevel{"LogLevel"};
    const std::string_view cSaveAutoDiscoverPSPVita{"AutoDiscoverPSPVita"};
    const std::string_view cSaveAutoDiscoverXLinkKai{"AutoDiscoverXLinkKai"};
    const std::string_view cSaveUseXLinkKaiHints{"UseXLinkKaiHints"};
    const std::string_view cSaveWifiAdapter{"WifiAdapter"};
    const std::string_view cSaveChannel{"Channel"};
    const std::string_view cSaveXLinkIp{"XLinkIp"};
    const std::string_view cSaveXLinkPort{"XLinkPort"};

    enum class EngineStatus
    {
        Idle = 0,
        Running,
        Error
    };

    static constexpr std::array<std::string_view, 3> cEngineStatusTexts{"Idle", "Running", "Error"};

    enum class Command
    {
        StartEngine = 0,
        StopEngine,
        StartSearchNetworks,
        StopSearchNetworks,
        SaveSettings,
        NoCommand
    };
}  // namespace WindowModel_Constants

class WindowModel
{
public:
    // Settings
    Logger::Level mLogLevel{Logger::Level::TRACE};
    bool          mAutoDiscoverPSPVitaNetworks{false};
    bool          mAutoDiscoverXLinkKaiInstance{false};
    bool          mXLinkKaiHints{false};
    std::string   mWifiAdapter{""};

    // Channel as a string because of the textfield this is bound to.
    std::string mChannel{"1"};
    std::string mXLinkIp{"127.0.0.1"};
    std::string mXLinkPort{"34523"};

    // Statuses
    WindowModel_Constants::EngineStatus mEngineStatus{WindowModel_Constants::EngineStatus::Idle};

    // Commands
    WindowModel_Constants::Command mCommand{WindowModel_Constants::Command::NoCommand};

    // Config

    /**
     * Saves the config in WindowModel to a file.
     * @param aPath - Path to save it in.
     * @return true if successful.
     */
    bool SaveToFile(std::string_view aPath) const;

    /**
     * Loads the config in a file to a WindowModel.
     * @param aPath - Path to save it in.
     * @return true if successful.
     */
    bool LoadFromFile(std::string_view aPath);
};
