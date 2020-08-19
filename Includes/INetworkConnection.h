#ifndef INETWORKCONNECTION_H
#define INETWORKCONNECTION_H

/* Copyright (c) 2020 [Rick de Bondt] - INetworkDevice.h
 *
 * This file contains an interface for the different network devices that can be used to as the promiscuous device.
 *
 * */

#include <string>


class INetworkConnection
{
public:
    /**
     * Creates a socket. \see https://man7.org/linux/man-pages/man2/socket.2.html
     * @param aDomain - for example AF_INET for IPv4.
     * @param aType - type of connection, for example SOCK_RAW.
     * @param aProtocol - Protocol to use, for example IPPROTO_UDP. \see https://man7.org/linux/man-pages/man7/ip.7.html
     * @return 0 if successful, errno if unsuccessful.
     */
    virtual int Socket(int aDomain, int aType, int aProtocol) = 0;

    /**
     * Gets the filedescriptor of the socket.
     * @return fd of the device, 0 on failure.
     */
    virtual int GetFd() = 0;

    /**
     * Closes the device
     * @return - 0 if successful, errno if unsuccessful.
     */
    virtual int Close() = 0;

    /**
     * Sends message on a socket \see https://man7.org/linux/man-pages/man2/send.2.html
     * @param aBuffer - The message.
     * @param aLength - The message length.
     * @param aFlags - Flags that change the behaviour of the send function.
     * @return The number of bytes sent, -1 if unsuccessful.
     */
    virtual int Send(const void* aBuffer, size_t aLength, int aFlags) = 0;

};


#endif //INETWORKCONNECTION_H
