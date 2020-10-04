#pragma once

namespace WindowModel_Constants
{
    enum class EngineStatus
    {
        Idle = 0,
        Running,
        Error
    };

    static const std::array<std::string, 3> cEngineStatusTexts{"Idle", "Running", "Error"};

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
    bool         mAutoDiscoverNetworks{false};
    bool         mXLinkKaiHints{false};
    bool         mAutoDiscoverXLinkKaiInstance{false};
    unsigned int mChannel{1};
    std::string  mXLinkIp{"127.0.0.1"};
    std::string  mXLinkPort{"34522"};

    // Statuses
    WindowModel_Constants::EngineStatus mEngineStatus{WindowModel_Constants::EngineStatus::Idle};

    // Commands
    WindowModel_Constants::Command mCommand{WindowModel_Constants::Command::NoCommand};
};
