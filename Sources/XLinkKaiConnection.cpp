#include "../Includes/XLinkKaiConnection.h"

#include <cstring>
#include <iostream>
#include <utility>

#include "../Includes/Logger.h"

using namespace boost::asio;

XLinkKaiConnection::~XLinkKaiConnection()
{
    Close();
}

bool XLinkKaiConnection::Open(std::shared_ptr<IPCapDevice> aPCapDevice, std::string_view aIp, unsigned int aPort)
{
    bool lReturn{true};

    mRemote = ip::udp::endpoint(ip::address::from_string(aIp.data()), aPort);

    try {
        mSocket.open(ip::udp::v4());
        mIp         = aIp;
        mPort       = aPort;
        mPCapDevice = std::move(aPCapDevice);

    } catch (const boost::system::system_error& lException) {
        Logger::GetInstance().Log("Failed to open socket: " + std::string(lException.what()), Logger::ERR);
        lReturn = false;
    }

    return lReturn;
}

bool XLinkKaiConnection::Connect()
{
    bool lReturn{true};

    if (Send(cConnectString)) {
        // Start the timer for receiving a confirmation from XLink Kai.
        mConnectInitiated = true;
        mConnectionTimerStart += (std::chrono::system_clock::now() - mConnectionTimerStart);
    } else {
        // Logging in send function
        lReturn = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::Send(std::string_view aMessage)
{
    bool lReturn{true};

    // We only allow connection/disconnection requests to be sent, when XLink Kai has not confirmed the connection yet.
    if (mSocket.is_open()) {
        if ((mConnected || aMessage == cConnectString || aMessage == cDisconnectString)) {
            try {
                Logger::GetInstance().Log("Sent: " + std::string(aMessage), Logger::TRACE);
                mSocket.send_to(buffer(std::string(aMessage)), mRemote);
            } catch (const boost::system::system_error& lException) {
                Logger::GetInstance().Log(
                    "Could not send message! " + std::string(aMessage) + std::string(lException.what()), Logger::ERR);
                lReturn = false;
            }
        } else {
            Logger::GetInstance().Log("No other messages before Xlink Kai has connected!", Logger::DEBUG);
            lReturn = false;
        }
    } else {
        Logger::GetInstance().Log("Could not send message on closed socket.", Logger::DEBUG);
        lReturn = false;
    }
    return lReturn;
}

bool XLinkKaiConnection::HandleKeepAlive()
{
    bool lReturn{true};
    if (!Send(cKeepAliveString)) {
        // Logging in send function.
        lReturn = false;
    }
    return lReturn;
}

void XLinkKaiConnection::ReceiveCallback(const boost::system::error_code& aError, size_t aBytesReceived)
{
    std::string lData{mData.begin(), mData.begin() + aBytesReceived};

    // If we actually received anything useful, react.
    if (!lData.empty()) {
        Logger::GetInstance().Log("Received: " + lData, Logger::TRACE);
        std::size_t lFirstSeparator{lData.find(cSeparator)};
        std::string lCommand{lData.substr(0, lFirstSeparator + 1)};


        if (!mConnected && (lCommand == std::string(cConnectedFormat) + cSeparator.data())) {
            lCommand = lData.substr(0, cConnectedString.size());
            if (lCommand == cConnectedString) {
                Logger::GetInstance().Log("XLink Kai succesfully connected: " + lCommand, Logger::INFO);
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
                    if (mPCapDevice != nullptr) {
                        // Strip e;e;
                        lData =
                            lData.substr(cEthernetDataString.length(), lData.length() - cEthernetDataString.length());
                        mPCapDevice->Send(lData);
                    }
                }
            } else if (lCommand == std::string(cDisconnectedFormat) + cSeparator.data()) {
                lCommand = lData.substr(0, cDisconnectedString.size());
                if (lCommand == cDisconnectedString) {
                    Logger::GetInstance().Log("Xlink Kai has disconnected us! " + lCommand, Logger::ERR);
                    mConnected = false;
                    mIoService.stop();
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
                    if ((!mConnected) && mConnectInitiated &&
                        (std::chrono::system_clock::now() > (mConnectionTimerStart + cConnectionTimeout))) {
                        Logger::GetInstance().Log("Timeout waiting for XLink Kai to connect", Logger::ERR);
                        mIoService.stop();
                        mConnectInitiated = false;
                        mConnected        = false;
                    }
                    mIoService.poll();

                    // Do not turn the computer into a toaster.
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            });
        }
    } else {
        Logger::GetInstance().Log("Can't start receiving without an opened socket!", Logger::ERR);
        lReturn = false;
    }

    return lReturn;
}

bool XLinkKaiConnection::Close()
{
    bool lReturn{true};

    try {
        if (mConnected || mConnectInitiated) {
            Send(cDisconnectString);
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
        lReturn = false;
    }

    return lReturn;
}

bool XLinkKaiConnection::IsDisconnected() const
{
    return (!mConnected);
}

bool XLinkKaiConnection::IsConnecting() const
{
    return (!mConnected && !mConnectInitiated);
}
