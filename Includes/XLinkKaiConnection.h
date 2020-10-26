#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - XLinkKaiConnection.h
 *
 * This file contains functions to talk to XLink Kai.
 *
 * */

#include <string>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "Handler8023.h"
#include "IConnector.h"
#include "IPCapDevice.h"

namespace XLinkKai_Constants
{
    static constexpr int                  cMaxLength{4096};
    static constexpr std::string_view     cIp{"127.0.0.1"};
    static constexpr std::string_view     cSeparator{";"};
    static constexpr std::string_view     cKeepAliveFormat{"keepalive"};
    static constexpr std::string_view     cConnectFormat{"connect"};
    static constexpr std::string_view     cConnectedFormat{"connected"};
    static constexpr std::string_view     cDisconnectFormat{"disconnect"};
    static constexpr std::string_view     cDisconnectedFormat{"disconnected"};
    static constexpr std::string_view     cEthernetDataFormat{"e"};
    static constexpr std::string_view     cLocallyUniqueName{"PSP"};
    static constexpr std::string_view     cEmulatorName{"Real_PSP"};
    static constexpr unsigned int         cPort{34523};
    static constexpr std::chrono::seconds cConnectionTimeout{10};

    static const std::string cConnectString{std::string(cConnectFormat) + cSeparator.data() +
                                            cLocallyUniqueName.data() + cSeparator.data() + cEmulatorName.data() +
                                            cSeparator.data()};

    static const std::string cConnectedString{std::string(cConnectedFormat) + cSeparator.data() +
                                              cLocallyUniqueName.data()};

    static const std::string cDisconnectedString{std::string(cDisconnectedFormat) + cSeparator.data() +
                                                 cLocallyUniqueName.data()};

    static const std::string cDisconnectString{std::string(cDisconnectFormat) + cSeparator.data()};

    static const std::string cKeepAliveString{std::string(cKeepAliveFormat) + cSeparator.data()};

    static const std::string cEthernetDataString{std::string(cEthernetDataFormat) + cSeparator.data() +
                                                 cEthernetDataFormat.data() + cSeparator.data()};
}  // namespace XLinkKai_Constants

using namespace XLinkKai_Constants;

/**
 * Class that connects to XLink Kai and sends and receives data from and to XLink Kai.
 */
class XLinkKaiConnection : public IConnector
{
public:
    XLinkKaiConnection() = default;
    ~XLinkKaiConnection();
    XLinkKaiConnection(const XLinkKaiConnection& aXLinkKaiConnection) = delete;
    XLinkKaiConnection& operator=(const XLinkKaiConnection& aXLinkKaiConnection) = delete;


    bool Open();


    bool Open(std::string_view aIp);

    /**
     * Creates a connection.
     * @param aIp - IP Address of the XLink Kai engine.
     * @param aPort - Port of the XLink Kai engine.
     * @return True if successful.
     */
    bool Open(std::string_view aIp, unsigned int aPort);

    /**
     * Connects to XLink Kai.
     * @return True if successful.
     */
    bool Connect();

    /**
     * Synchronous receive of network messages from XLink Kai, may hang if nothing received!.
     * @return True if successful.
     */
    bool ReadNextData() override;

    bool StartReceiverThread() override;

    /**
     * Sends a message to Xlink Kai.
     * @param aCommand - Command that should be added to the XLink Kai message (for example connect).
     * @param aData - Data to be sent to XLink Kai.
     * @return True if successful.
     */
    bool Send(std::string_view aCommand, std::string_view aData);

    bool Send(std::string_view aData) override;

    void Close() final;

    /**
     * Sets port to XLink Kai interface.
     * @param aPort - Port to connect to.
     */
    void SetPort(unsigned int aPort);

    void SetIncomingConnection(std::shared_ptr<IPCapDevice> aDevice) override;

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

    std::array<char, cMaxLength> mData{};
    // Raw ethernet data received from XLink Kai
    std::string                    mEthernetData{};
    std::shared_ptr<IPCapDevice>   mIncomingConnection{nullptr};
    std::string                    mIp{cIp};
    boost::asio::io_service        mIoService{};
    Handler8023                    mPacketHandler{};
    unsigned int                   mPort{cPort};
    std::shared_ptr<boost::thread> mReceiverThread{nullptr};
    boost::asio::ip::udp::endpoint mRemote{};
    boost::asio::ip::udp::socket   mSocket{mIoService};
};
