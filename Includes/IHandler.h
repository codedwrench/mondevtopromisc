#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IHandler.h
 *
 * This file contains an interface for packet handlers.
 *
 **/

#include <string_view>

#include <bits/stdint-uintn.h>

class IHandler
{
public:
    /**
     * Gets destination MAC address of packet.
     * @return the destination MAC address.
     */
    [[nodiscard]] virtual uint64_t GetDestinationMAC() const = 0;

    /**
     * Gets packet saved in this class.
     * @return string_view with data.
     */
    virtual std::string_view GetPacket() = 0;

    /**
     * Gets source MAC address of packet.
     * @return the source MAC address.
     */
    [[nodiscard]] virtual uint64_t GetSourceMAC() const = 0;

    /**
     * Preload data about this packet into this class.
     * @param aData - Packet to dissect.
     */
    virtual void Update(std::string_view aPacket) = 0;
};