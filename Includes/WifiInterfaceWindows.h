#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - WifiInterfaceWindows.h
 *
 * This file contains Windows specific functions for managing WiFi adapters.
 *
 **/

#include <string>
#include <vector>

#include <guiddef.h>

#include "IWifiInterface.h"

typedef void* HANDLE;

/**
 * Class that manages WiFi adapters.
 */
class WifiInterface : public IWifiInterface
{
public:
    WifiInterface(std::string_view aAdapterName);
    ~WifiInterface();
    bool                          Connect(const WifiInformation& aConnection) override { return true; }
    uint64_t                      GetAdapterMACAddress() override;
    std::vector<WifiInformation>& GetAdhocNetworks() override;
    bool                          LeaveIBSS() override { return true; }

private:
    std::vector<WifiInformation> mLastReceivedScanInformation;
    std::string                  mAdapterName;
    GUID                         mGUID{};
    HANDLE                       mWifiHandle{nullptr};
};
