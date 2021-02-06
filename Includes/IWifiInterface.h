#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

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
    struct WifiInformation
    {
        std::string            ssid;
        std::array<uint8_t, 6> bssid;
        int                    frequency;
        bool                   isadhoc;
        bool                   isconnected;
    };

    /**
     * Connects to a wireless network.
     * @param aConnection - Network to connect to.
     * @return true if successful.
     */
    virtual bool Connect(const WifiInformation& aConnection) = 0;

    /**
     * Get the MAC address of the adapter.
     * @return the MAC address of the adapter.
     */
    virtual uint64_t GetAdapterMACAddress() = 0;

    /**
     * Gets the adhoc networks the network adapter has found.
     * @return a list of adhoc networks, an empty list if none found.
     */
    virtual std::vector<WifiInformation>& GetAdhocNetworks() = 0;
};
