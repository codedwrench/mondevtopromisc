#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../Includes/XLinkKaiConnection.h"
#include "../Includes/Logger.h"

// TODO: Not happy with this, should be some common file with all the linux function wrappers.
int XLinkKaiConnection::Socket(int aDomain, int aType, int aProtocol)
{
    return socket(aDomain, aType, aProtocol);
}

int XLinkKaiConnection::Bind(const sockaddr* aAddress, socklen_t aAddressLength)
{
    return bind(mFd, aAddress, aAddressLength);
}

XLinkKaiConnection::~XLinkKaiConnection()
{
    if (mFd != 0) {
        Close();
    }
}

bool XLinkKaiConnection::Open(const std::string& aIp, unsigned int aPort)
{
    bool lReturn = false;

    // Create a UDP socket for Xlink Kai.
    int lFd = Socket(AF_INET, SOCK_DGRAM, 0);
    if (lFd != -1) {
        mFd = lFd;

        sockaddr_in lAddress{};
        memset(static_cast<void*>(&lAddress), 0, sizeof(lAddress));

        inet_pton(AF_INET, aIp.c_str(), &(lAddress));
        lAddress.sin_family = AF_INET;
        lAddress.sin_port = aPort;

        if (Bind(reinterpret_cast<sockaddr*>(&lAddress), sizeof(lAddress)) == 0) {
            Logger::GetInstance().Log("Connected!", Logger::DEBUG);
            lReturn = true;
        } else {
            Logger::GetInstance().Log("Failed to bind socket: " + std::string(strerror(errno)), Logger::ERROR);
        }
    } else {
        Logger::GetInstance().Log("Failed to open socket: " + std::string(strerror(errno)), Logger::ERROR);
    }

    return lReturn;
}