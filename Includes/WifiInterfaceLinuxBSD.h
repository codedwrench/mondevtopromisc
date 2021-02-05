#pragma once

#include "IWifiInterface.h"

/* Copyright (c) 2020 [Rick de Bondt] - WifiInterfaceLinuxBSD.h
 *
 * This file contains Linux and BSD specific functions for managing WiFi adapters.
 *
 **/

#include <ifaddrs.h>

#ifdef __linux__
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#else
#include <net/if_dl.h>
#endif

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
};
