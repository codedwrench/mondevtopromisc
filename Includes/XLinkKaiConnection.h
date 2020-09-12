#ifndef XLINKKAICONNECTION_H
#define XLINKKAICONNECTION_H

/* Copyright (c) 2020 [Rick de Bondt] - XLinkKaiConnection.h
 *
 * This file contains functions to talk to XLink Kai.
 *
 * */

#include <string>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "IPCapDevice.h"

namespace XLinkKai_Constants
{
    constexpr int                  cMaxLength{4096};
    constexpr std::string_view     cIp{"127.0.0.1"};
    constexpr std::string_view     cSeparator{";"};
    constexpr std::string_view     cKeepAliveFormat{"keepalive"};
    constexpr std::string_view     cConnectFormat{"connect"};
    constexpr std::string_view     cConnectedFormat{"connected"};
    constexpr std::string_view     cDisconnectFormat{"disconnect"};
    constexpr std::string_view     cDisconnectedFormat{"disconnected"};
    constexpr std::string_view     cEthernetDataFormat{"e"};
    constexpr std::string_view     cLocallyUniqueName{"PSP"};
    constexpr std::string_view     cEmulatorName{"Real_PSP"};
    constexpr unsigned int         cPort{34523};
    constexpr std::chrono::seconds cConnectionTimeout{10};

    const std::string cConnectString{std::string(cConnectFormat) + cSeparator.data() + cLocallyUniqueName.data() +
                                     cSeparator.data() + cEmulatorName.data() + cSeparator.data()};

    const std::string cConnectedString{std::string(cConnectedFormat) + cSeparator.data() + cLocallyUniqueName.data()};

    const std::string cDisconnectedString{std::string(cDisconnectedFormat) + cSeparator.data() +
                                          cLocallyUniqueName.data()};

    const std::string cDisconnectString{std::string(cDisconnectFormat) + cSeparator.data()};

    const std::string cKeepAliveString{std::string(cKeepAliveFormat) + cSeparator.data()};

    const std::string cEthernetDataString{std::string(cEthernetDataFormat) + cSeparator.data() +
                                          cEthernetDataFormat.data() + cSeparator.data()};
}  // namespace XLinkKai_Constants

using namespace XLinkKai_Constants;

/**
 * Class that connects to XLink Kai and sends and receives data from and to XLink Kai.
 */
class XLinkKaiConnection
{
public:
    XLinkKaiConnection() = default;
    ~XLinkKaiConnection();
    XLinkKaiConnection(const XLinkKaiConnection& aXLinkKaiConnection) = delete;
    XLinkKaiConnection& operator=(const XLinkKaiConnection& aXLinkKaiConnection) = delete;

    /**
     * Creates a connection.
     * @param aPCapDevice - Pointer to a pcap device that will be able to send packets. (optional)
     * @param aIp - IP Address of the XLink Kai engine.
     * @param aPort - Port of the XLink Kai engine.
     * @return True if successful.
     */
    bool Open(std::shared_ptr<IPCapDevice> aPCapDevice = nullptr,
              std::string_view             aIp         = cIp,
              unsigned int                 aPort       = cPort);

    /**
     * Connects to XLink Kai.
     * @return True if successful.
     */
    bool Connect();

    /**
     * Starts receiving network messages from Xlink Kai.
     * @return True if successful.
     */
    bool StartReceiverThread();

    /**
     * Sends a message to Xlink Kai.
     * @return True if successful.
     */
    bool Send(std::string_view aMessage);

    /**
     * Check if XLink Kai connection has been interrupted.
     * @return True if disconnected.
     */
    [[nodiscard]] bool IsDisconnected() const;

    /**
     * Check if XLink Kai is connecting.
     * @return True if connecting.
     */
    [[nodiscard]] bool IsConnecting() const;

    /**
     * Closes the connection, this function should not throw exceptions! As it is used in a destructor.
     * @return True if successful.
     */
    bool Close();

private:
    /**
     * Handles traffic from XLink Kai.
     */
    void ReceiveCallback(const boost::system::error_code& aError, size_t aBytesReceived);

    /**
     * Sends a keepalive back to the XLink Kai engine, call this function when a keepalive is received.
     * @return True if all bytes have been sent over successfully.
     */
    bool HandleKeepAlive();

    bool                                               mConnected{false};
    bool                                               mConnectInitiated{false};
    std::chrono::time_point<std::chrono::system_clock> mConnectionTimerStart{std::chrono::seconds{0}};

    std::array<char, cMaxLength>   mData{};
    std::string                    mIp{cIp};
    unsigned int                   mPort{cPort};
    boost::asio::io_service        mIoService{};
    boost::asio::ip::udp::socket   mSocket{mIoService};
    boost::asio::ip::udp::endpoint mRemote{};
    std::shared_ptr<boost::thread> mReceiverThread{nullptr};
    std::shared_ptr<IPCapDevice>   mPCapDevice{nullptr};
};

#endif  // XLINKKAICONNECTION_H
