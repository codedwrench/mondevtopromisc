#pragma once

#include "IWifiInterface.h"

/* Copyright (c) 2020 [Rick de Bondt] - WifiInterfaceLinuxBSD.h
 *
 * This file contains Linux and BSD specific functions for managing WiFi adapters.
 *
 **/

#include <array>
#include <string_view>

#include <linux/nl80211.h>

#include <netlink/errno.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/netlink.h>

#include "Parameter80211Reader.h"

struct nl_sock;

/**
 * Class that manages WiFi adapters.
 */
namespace WifiInterface_Constants
{
    static constexpr std::string_view cDriverName{"nl80211"};
    static constexpr std::string_view cScanCommand{"scan"};
    static constexpr std::string_view cControlCommand{"nlctrl"};
    struct TriggerResults
    {
        int done;
        int aborted;
    };

    struct HandlerArguments
    {  // For FamilyHandler() and nl_get_multicast_id().
        std::string group;
        int         id;
    };

    struct DumpResultArgument
    {
        std::array<nla_policy, NL80211_BSS_MAX + 1>& bssserviceinfo;
        std::vector<std::string>&                    adhocnetworks;
    };

}  // namespace WifiInterface_Constants
class WifiInterface : public IWifiInterface
{
public:
    explicit WifiInterface(std::string_view aAdapterName);
    ~WifiInterface();
    uint64_t                 GetAdapterMACAddress() override;
    std::vector<std::string> GetAdhocNetworks() override;

private:
    int  ScanTrigger();
    int  GetMulticastId();
    void SetBSSPolicy();

    std::string                                 mAdapterName;
    std::array<nla_policy, NL80211_BSS_MAX + 1> mBSSPolicy;
    nl_sock*                                    mSocket{nullptr};
    int                                         mDriverId{0};
    unsigned int                                mNetworkAdapterIndex{0};
    Parameter80211Reader                        mReader{nullptr};
};
