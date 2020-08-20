#include <cstring>
#include "../Includes/XLinkKaiConnection.h"
#include "../Includes/Logger.h"

using namespace boost::asio;

bool XLinkKaiConnection::Open(std::string_view aIp, unsigned int aPort)
{
    bool lReturn{true};

    mRemote = ip::udp::endpoint(ip::address::from_string(aIp.data()), aPort);

    try {
        // Open the socket
        mSocket.open(ip::udp::v4());

    } catch (const boost::system::system_error& lException) {
        Logger::GetInstance().Log("Failed to open socket: " + std::string(lException.what()), Logger::ERROR);
        lReturn = false;
    }

    return lReturn;
}

bool XLinkKaiConnection::Connect()
{
    bool lReturn{true};
    if (mSocket.is_open()) {
        try {
            mSocket.send_to(buffer(cConnectString), mRemote);

            std::array<char, 15> lReceiveBuffer{}; // connected;PSP
            mSocket.receive_from(buffer(lReceiveBuffer), mRemote);
            std::string lReceivedData = lReceiveBuffer.data();
            if (lReceivedData != cConnectedString) {
                Logger::GetInstance().Log("Received invalid confirmation! " + lReceivedData, Logger::ERROR);
                lReturn = false;
            }

        } catch (const boost::system::system_error& lException) {
            Logger::GetInstance().Log("Could not connect!" + std::string(lException.what()), Logger::ERROR);
            lReturn = false;
        }
    }
    return lReturn;
}

bool XLinkKaiConnection::HandleKeepAlive()
{
    bool lReturn{true};
    if (mSocket.is_open()) {
        try {
            mSocket.send_to(buffer(cKeepAliveString), mRemote);
            Logger::GetInstance().Log("Sent keepalive.", Logger::DEBUG);
        } catch (const boost::system::system_error& lException) {
            Logger::GetInstance().Log("Could not send keepalive.", Logger::DEBUG);
            lReturn = false;
        }
    }
    return lReturn;
}

void XLinkKaiConnection::ReceiveCallback(const boost::system::error_code& aError, size_t aBytesReceived)
{
    std::string lData{mData.data()};
    std::size_t lFirstSeparator{lData.find(cSeparator)};
    std::string lCommand{lData.substr(0, lFirstSeparator + 1)};

    if (lCommand == cKeepAliveString) {
        HandleKeepAlive();
    } else if (lCommand == std::string(cEthernetDataFormat) + cSeparator.data()) {
        // For data XLink Kai uses e;e; which doesn't filter all that well, so if we find e; just check if this
        // is e;e;
        lCommand = lData.substr(0, cEthernetDataString.size());
        if (lCommand != cEthernetDataString) {
            Logger::GetInstance().Log("Unknown command received: " + lCommand, Logger::DEBUG);
            lCommand = "";
        } else {
            Logger::GetInstance().Log("Data: " + lData.substr(cEthernetDataString.size(), lData.size()), Logger::DEBUG);
        }
    }
    StartReceiverThread();
}

bool XLinkKaiConnection::StartReceiverThread()
{
    bool lReturn{true};
    if (mSocket.is_open()) {
        mSocket.async_receive_from(
                buffer(mData, cMaxLength), mRemote,
                boost::bind(&XLinkKaiConnection::ReceiveCallback,
                            this,
                            placeholders::error,
                            placeholders::bytes_transferred));
        // Run
        if (mReceiverThread == nullptr) {
            mReceiverThread = std::make_shared<boost::thread>([lIoService = &mIoService] { lIoService->run(); });
        }
    } else {
        Logger::GetInstance().Log("Can't start receiving without an opened socket!", Logger::ERROR);
        lReturn = false;
    }

    return lReturn;
}