#pragma once

#include <array>
#include <string>

#include "../Includes/Logger.h"

namespace WindowModel_Constants
{
    static constexpr std::string_view cSaveFilePath{"config.txt"};

    static constexpr std::string_view cSaveLogLevel{"LogLevel"};
    static constexpr std::string_view cSaveAutoDiscoverPSPVita{"AutoDiscoverPSPVita"};
    static constexpr std::string_view cSaveAutoDiscoverXLinkKai{"AutoDiscoverXLinkKai"};
    static constexpr std::string_view cSaveUseXLinkKaiHints{"UseXLinkKaiHints"};
    static constexpr std::string_view cSaveWifiAdapter{"WifiAdapter"};
    static constexpr std::string_view cSaveChannel{"Channel"};
    static constexpr std::string_view cSaveXLinkIp{"XLinkIp"};
    static constexpr std::string_view cSaveXLinkPort{"XLinkPort"};
    static constexpr std::string_view cSaveAcknowledgeDataFrames{"AckDataFrames"};
    static constexpr std::string_view cSaveOnlyAcceptFromMac{"OnlyAcceptFromMac"};

    static constexpr Logger::Level    cDefaultLogLevel{Logger::Level::ERROR};
    static constexpr bool             cDefaultAutoDiscoverPSPVita{false};
    static constexpr bool             cDefaultAutoDiscoverXLinkKai{false};
    static constexpr bool             cDefaultUseXLinkKaiHints{false};
    static constexpr std::string_view cDefaultChannel{"1"};
    static constexpr std::string_view cDefaultWifiAdapter{""};
    static constexpr std::string_view cDefaultXLinkIp{"127.0.0.1"};
    static constexpr std::string_view cDefaultXLinkPort{"34523"};
    static constexpr std::string_view cDefaultAcknowledgeDataFrames{"AckDataFrames"};
    static constexpr std::string_view cDefaultOnlyAcceptFromMac{"OnlyAcceptFromMac"};

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
    Logger::Level mLogLevel{WindowModel_Constants::cDefaultLogLevel};
    bool          mAutoDiscoverPSPVitaNetworks{WindowModel_Constants::cDefaultAutoDiscoverPSPVita};
    bool          mAutoDiscoverXLinkKaiInstance{WindowModel_Constants::cDefaultAutoDiscoverXLinkKai};
    bool          mXLinkKaiHints{WindowModel_Constants::cDefaultUseXLinkKaiHints};
    std::string   mWifiAdapter{WindowModel_Constants::cDefaultWifiAdapter};
    bool          mAcknowledgeDataFrames{false};
    std::string   mOnlyAcceptFromMac{};

    // Channel as a string because of the textfield this is bound to.
    std::string mChannel{WindowModel_Constants::cDefaultChannel};
    std::string mXLinkIp{WindowModel_Constants::cDefaultXLinkIp};
    std::string mXLinkPort{WindowModel_Constants::cDefaultXLinkPort};

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
