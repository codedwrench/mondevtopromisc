#ifndef XLINKKAICONNECTION_H
#define XLINKKAICONNECTION_H

/* Copyright (c) 2020 [Rick de Bondt] - XLinkKaiConnection.h
 *
 * This file contains functions to talk to XLink Kai.
 *
 * */

#include "INetworkConnection.h"
#include <string>
#include <unistd.h>

namespace XLinkKai_Constants
{
    constexpr char cIp[]{"127.0.0.1"};
    constexpr char cLocalIdentifier[]{""};
    constexpr unsigned int cPort{34523};
}

using namespace XLinkKai_Constants;

class XLinkKaiConnection : public INetworkConnection
{
public:
    XLinkKaiConnection() = default;
    XLinkKaiConnection(const XLinkKaiConnection& aXLinkKaiConnection) = delete;
    XLinkKaiConnection& operator=(const XLinkKaiConnection& aXLinkKaiConnection) = delete;
    ~XLinkKaiConnection();

    int GetFd() override
    { return 0; };

    int Send(const void* aBuffer, size_t aLength, int aFlags) override
    { return 0; };

    int Socket(int aDomain, int aType, int aProtocol) override;

    int Bind(const sockaddr* aAddress, socklen_t aAddressLength);

    int Close() final
    {
        close(mFd);
        return 0;
    };

    /**
     * Creates a connection.
     * @param aIp - IP Address of the XLink Kai engine.
     * @return True if successful.
     */
    bool Open(const std::string& aIp = cIp, unsigned int aPort = cPort);
private:
    std::string mIp{cIp};
    unsigned int mPort{cPort};
    int mFd{0};
};

#endif // XLINKKAICONNECTION_H
