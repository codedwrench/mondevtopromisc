
/* Copyright (c) 2020 [Rick de Bondt] - ILinuxDevice.h
 *
 * This file contains an interface for pcap devices, either file based or device based.
 *
 * */

#ifndef IPCAPDEVCE_H
#define IPCAPDEVCE_H

#include <string>
#include <pcap.h>

class IPCapDevice {
public:

    /**
     * Closes a pcap device/file.
     */
    virtual void Close() = 0;

    /**
     * Opens the device/file.
     * @param aName - the name of the device to capture or the path of the file to open.
     * @return true on success.
     */
    virtual bool Open(const std::string &aName) = 0;

    /**
     * Reads the next packet from the device or file.
     * @return true on success.
     */
    virtual bool ReadNextPacket() = 0;

    /**
     * Gets data from last read packet.
     * @return pointer to data as an unsigned char array.
     */
    virtual const unsigned char *GetData() = 0;

    /**
     * Gets header from last read packet.
     * @return pointer to header as pcap_pkthdr type.
     */
    virtual const pcap_pkthdr *GetHeader() = 0;
};


#endif //IPCAPDEVICE_H
