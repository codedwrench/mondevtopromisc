#ifndef WINDOWMODEL_H
#define WINDOWMODEL_H

namespace WindowModel_Constants
{
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

using namespace WindowModel_Constants;

class WindowModel
{
public:
    bool         mAutoDiscoverNetworks;
    bool         mXLinkKaiHints;
    bool         mAutoDiscoverXLinkKaiInstance;
    unsigned int mChannel;

    Command mCommand;
};

#endif  // WINDOWMODEL_H
