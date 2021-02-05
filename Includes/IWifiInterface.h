#pragma once

#include <string>
#include <vector>

#include <bits/stdint-uintn.h>

/* Copyright (c) 2020 [Rick de Bondt] - IWifiInterface.h
 *
 * This file contains an interface for wifi devices.
 *
 **/


/**
 * Interface for wifi devices.
 */
class IWifiInterface
{
public:
    /**
     * Get the MAC address of the adapter.
     * @return the MAC address of the adapter.
     */
    virtual uint64_t GetAdapterMACAddress() = 0;

    /**
     * Gets the adhoc networks the network adapter has found.
     * @return a list of adhoc networks, an empty list if none found.
     */
    virtual std::vector<std::string> GetAdhocNetworks() = 0;
};
