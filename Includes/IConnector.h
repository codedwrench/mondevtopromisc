#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IConnector.h
 *
 * This file contains an interface for outside connections, either file based or device based.
 *
 **/

#include <memory>
#include <string>

class IPCapDevice;

/**
 * Interface for outside connections.
 */
class IConnector
{
public:
    /**
     * Closes the device/file.
     */
    virtual void Close() = 0;

    /**
     * Reads the next data from the device or file.
     * @return true on success.
     */
    virtual bool ReadNextData() = 0;

    /**
     * Sends data over device/file if supported.
     * @param aData - Data to send.
     * @return true if successful, false on failure or unsupported.
     */
    virtual bool Send(std::string_view aData) = 0;

    /**
     * Allows sending over different device.
     * @param aDevice - Device to use.
     */
    virtual void SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice) = 0;

    /**
     * Starts receiving network messages from Connector.
     * @return True if successful.
     */
    virtual bool StartReceiverThread() = 0;
};
