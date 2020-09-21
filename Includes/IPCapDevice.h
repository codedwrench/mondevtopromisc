#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IPCapDevice.h
 *
 * This file contains an interface for pcap devices, either file based or device based.
 *
 **/

#include <string>

#include <pcap/pcap.h>

#include "ISendReceiveDevice.h"

/**
 * Interface for pcap devices, either file based or device based.
 */
class IPCapDevice : public ISendReceiveDevice
{
public:
    /**
     * Returns data as string.
     * @param aData - Data from pcap functions
     * @param aHeader - Header from pcap functions
     * @return Data as string
     */
    virtual std::string DataToString(const unsigned char* aData, const pcap_pkthdr* aHeader) = 0;

    /**
     * Gets data from last read packet.
     * @return pointer to data as an unsigned char array.
     */
    virtual const unsigned char* GetData() = 0;

    /**
     * Gets header from last read packet.
     * @return pointer to header as pcap_pkthdr type.
     */
    virtual const pcap_pkthdr* GetHeader() = 0;

    /**
     * Set BSSID to specified BSSID, this is used for sending packets and filtering.
     * @return true on success.
     */
    virtual void SetBSSID(std::string_view aBssid) = 0;
};
