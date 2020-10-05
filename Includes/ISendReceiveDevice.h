#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - ISendReceiveDevice.h
 *
 * This file contains an interface for devices that can receive or send data.
 *
 **/

#include <memory>
#include <string>

/**
 * Interface for devices that can receive or send data.
 */
class ISendReceiveDevice
{
public:
    /**
     * Closes the device/file.
     */
    virtual void Close() = 0;

    /**
     * Returns last received data as string.
     * @return a string containing the data.
     */
    virtual std::string LastDataToString() = 0;

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
     * Allows sending or receiving over different device.
     * @param aDevice - Device to use.
     */
    virtual void SetSendReceiveDevice(std::shared_ptr<ISendReceiveDevice> aDevice) = 0;
};
