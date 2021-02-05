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
    uint64_t                 GetAdapterMACAddress() override;
    std::vector<std::string> GetAdhocNetworks() override;

private:
    std::string mAdapterName;
    GUID        mGUID{};
    HANDLE      mWifiHandle{nullptr};
};
