#include "../Includes/XLinkKaiConnection.h"

/* Copyright (c) 2020 [Rick de Bondt] - XLinkKaiConnection.cpp */

#include <cstring>
#include <iostream>
#include <utility>

#include "../Includes/Logger.h"
#include "../Includes/MonitorDevice.h"

using namespace boost::asio;

XLinkKaiConnection::~XLinkKaiConnection()
{
    Close();
}

bool XLinkKaiConnection::Open()
{
    return Open("", 0);
}

bool XLinkKaiConnection::Open(std::string_view aIp)
{
    return Open(aIp, cPort);
}

bool XLinkKaiConnection::Open(std::string_view aIp, unsigned int aPort)
{
    bool lReturn{true};

    std::string  lIp{aIp};
    unsigned int lPort{aPort};

    // TODO: Do broadcast, but for now, use default stuff
    if (aIp == "") {
        lIp   = cIp;
        lPort = cPort;
    }

    mRemote = ip::udp::endpoint(ip::address::from_string(aIp.data()), aPort);

    try {
        mSocket.open(ip::udp::v4());
        mIp   = aIp;
        mPort = aPort;
    } catch (const boost::system::system_error& lException) {
        Logger::GetInstance().Log("Failed to open socket: " + std::string(lException.what()), Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}

bool XLinkKaiConnection::Connect()
{
    bool lReturn{true};

    if (Send(cConnectString, "")) {
        // Start the timer for receiving a confirmation from XLink Kai.
        mConnectInitiated = true;
        mConnectionTimerStart += (std::chrono::system_clock::now() - mConnectionTimerStart);
    } else {
        // Logging in send function
        lReturn = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::Send(std::string_view aCommand, std::string_view aData)
{
    bool lReturn{true};

    // We only allow connection/disconnection requests to be sent, when XLink Kai has not confirmed the connection yet.
    if (mSocket.is_open()) {
        if ((mConnected || aCommand == cConnectString || aCommand == cDisconnectString)) {
            try {
                Logger::GetInstance().Log("Sent: " + std::string(aCommand) + std::string(aData), Logger::Level::TRACE);
                mSocket.send_to(buffer(std::string(aCommand) + std::string(aData)), mRemote);
            } catch (const boost::system::system_error& lException) {
                Logger::GetInstance().Log(
                    "Could not send message! " + std::string(aData) + std::string(lException.what()),
                    Logger::Level::ERROR);
                lReturn = false;
            }
        } else {
            Logger::GetInstance().Log("No other messages before Xlink Kai has connected!", Logger::Level::DEBUG);
            lReturn = false;
        }
    } else {
        Logger::GetInstance().Log("Could not send message on closed socket.", Logger::Level::DEBUG);
        lReturn = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::Send(std::string_view aData)
{
    return Send(cEthernetDataString, aData);
}

bool XLinkKaiConnection::HandleKeepAlive()
{
    bool lReturn{true};
    if (!Send(cKeepAliveString, "")) {
        // Logging in send function.
        lReturn = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::ReadNextData()
{
    bool lReturn{true};

    size_t lBytesReceived{mSocket.receive_from(buffer(mData, cMaxLength), mRemote)};

    if (lBytesReceived > 0) {
        ReceiveCallback(boost::system::error_code(), lBytesReceived);
    }

    return lReturn;
}

void XLinkKaiConnection::ReceiveCallback(const boost::system::error_code& aError, size_t aBytesReceived)
{
    std::string lData{mData.begin(), mData.begin() + aBytesReceived};
    mPacketHandler.Update(lData);

    // If we actually received anything useful, react.
    if (!lData.empty()) {
        Logger::GetInstance().Log("Received: " + lData, Logger::Level::TRACE);
        std::size_t lFirstSeparator{lData.find(cSeparator)};
        std::string lCommand{lData.substr(0, lFirstSeparator + 1)};


        if (!mConnected && (lCommand == std::string(cConnectedFormat) + cSeparator.data())) {
            lCommand = lData.substr(0, cConnectedString.size());
            if (lCommand == cConnectedString) {
                Logger::GetInstance().Log("XLink Kai succesfully connected: " + lCommand, Logger::Level::INFO);
                mConnectInitiated = false;
                mConnected        = true;
            }
        }

        // If no connection confirmation has been sent on XLink Kai's side, Don't care about any other message yet
        if (mConnected) {
            if (lCommand == cKeepAliveString) {
                HandleKeepAlive();
            } else if (lCommand == std::string(cEthernetDataFormat) + cSeparator.data()) {
                // For data XLink Kai uses e;e; which doesn't filter all that well, so if we find e; just check if this
                // is e;e;
                lCommand = lData.substr(0, cEthernetDataString.size());
                if (lCommand == cEthernetDataString) {
                    if (mIncomingConnection != nullptr) {
                        // Strip e;e;
                        mEthernetData = lData.substr(cEthernetDataString.length(), lData.length() - cEthernetDataString.length());
                        lData = mEthernetData;

                        std::shared_ptr<MonitorDevice> lMonitorDevice = std::dynamic_pointer_cast<MonitorDevice>(mIncomingConnection);

                        // If it is actually a monitor device, do convert.
                        if(lMonitorDevice != nullptr) {
                            lData = mPacketHandler.ConvertPacket(lMonitorDevice->GetLockedBSSID(), lMonitorDevice->GetDataPacketParameters());

                            // Data from XLink Kai should never be caught in the receiver thread of the Monitor device.
                            lMonitorDevice->BlackList(mPacketHandler.GetSourceMAC());
                        }
                        mIncomingConnection->Send(lData);
                    }
                }
            } else if (lCommand == std::string(cDisconnectedFormat) + cSeparator.data()) {
                lCommand = lData.substr(0, cDisconnectedString.size());
                if (lCommand == cDisconnectedString) {
                    Logger::GetInstance().Log("Xlink Kai has disconnected us! " + lCommand, Logger::Level::ERROR);
                    mConnected = false;
                }
            }
        }
    }

    StartReceiverThread();
}

bool XLinkKaiConnection::StartReceiverThread()
{
    bool lReturn{true};
    if (mSocket.is_open()) {
        mSocket.async_receive_from(
            buffer(mData, cMaxLength),
            mRemote,
            boost::bind(
                &XLinkKaiConnection::ReceiveCallback, this, placeholders::error, placeholders::bytes_transferred));
        // Run
        if (mReceiverThread == nullptr) {
            mReceiverThread = std::make_shared<boost::thread>([&] {
                mIoService.restart();
                while (!mIoService.stopped()) {
                    if ((!mConnected && !mConnectInitiated)) {
                        // Lost connection somewhere, reconnect.
                        Connect();
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    } else if ((!mConnected) && mConnectInitiated &&
                               (std::chrono::system_clock::now() > (mConnectionTimerStart + cConnectionTimeout))) {
                        Logger::GetInstance().Log("Timeout waiting for XLink Kai to connect", Logger::Level::ERROR);
                        mIoService.stop();
                        mConnectInitiated = false;
                        mConnected        = false;
                    } else {
                        mIoService.poll();
                        std::this_thread::sleep_for(std::chrono::microseconds(1));
                    }
                }
            });
        }
    } else {
        Logger::GetInstance().Log("Can't start receiving without an opened socket!", Logger::Level::ERROR);
        lReturn = false;
    }

    return lReturn;
}

void XLinkKaiConnection::Close()
{
    try {
        if (mConnected || mConnectInitiated) {
            Send(cDisconnectString, "");
            mConnected        = false;
            mConnectInitiated = false;
        }

        if (mReceiverThread != nullptr) {
            if (!mIoService.stopped()) {
                mIoService.stop();
            }
            mReceiverThread->join();
            mReceiverThread = nullptr;
        }

        if (mSocket.is_open()) {
            mSocket.close();
        }
    } catch (...) {
        std::cout << "Failed to disconnect :( " + boost::current_exception_diagnostic_information() << std::endl;
    }
}

void XLinkKaiConnection::SetPort(unsigned int aPort)
{
    mPort = aPort;
}

void XLinkKaiConnection::SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice)
{
    mIncomingConnection = aDevice;
}
