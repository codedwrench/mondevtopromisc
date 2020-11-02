#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IHandler.h
 *
 * This file contains an interface for packet handlers.
 *
 **/

#include <string_view>

class IHandler
{
public:
    /**
     * Add the source MAC address to blacklist.
     * @param aMac - MAC address to blacklist.
     */
    virtual void AddToMACBlackList(uint64_t aMAC) = 0;

    /**
     * Add the source MAC address to whitelist. Whitelist takes prevalence over the blacklist.
     * @param aMac - MAC address to whitelist.
     */
    virtual void AddToMACWhiteList(uint64_t aMAC) = 0;

    /**
     * Clears blacklist.
     */
    virtual void ClearMACBlackList() = 0;

    /**
     * Clears whitelist.
     */
    virtual void ClearMACWhiteList() = 0;

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
     * Checks if MAC is in receiver blacklist.
     * @param aMAC - MAC to check
     * @return true if MAC is blacklisted.
     */
    [[nodiscard]] virtual bool IsMACBlackListed(uint64_t aMAC) const = 0;

    /**
     * Checks if this MAC is not blacklisted / whitelisted.
     * @param aMAC - MAC to check
     * @return true if MAC address is allowed.
     */
    virtual bool IsMACAllowed(uint64_t aMAC) = 0;

    /**
     * Preload data about this packet into this class.
     * @param aData - Packet to dissect.
     */
    virtual void Update(std::string_view aPacket) = 0;
};