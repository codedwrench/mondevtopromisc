
#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

WifiInterface::WifiInterface(std::string_view aAdapter)
{
    mAdapterName = aAdapter;
}

WifiInterface::~WifiInterface()
{

}

std::vector<std::string> WifiInterface::GetAdhocNetworks()
{
    // Declare and initialize variables.
    std::vector<std::string> lReturn{};

            Logger::GetInstance().Log("Not implemented yet!",
                                      Logger::Level::DEBUG);

    return lReturn;
}

uint64_t WifiInterface::GetAdapterMACAddress()
{
    uint64_t lReturn{0};

    ifaddrs* lInterfaceAddresses{nullptr};
    ifaddrs* lInterfaceAddress{nullptr};

    if (getifaddrs(&lInterfaceAddresses) == 0) {
        for (lInterfaceAddress = lInterfaceAddresses; (lInterfaceAddress != nullptr) && (lReturn == 0);
             lInterfaceAddress = lInterfaceAddress->ifa_next) {
#ifdef __linux__
            if ((lInterfaceAddress->ifa_name == mAdapterName) &&
                (lInterfaceAddress->ifa_addr->sa_family == AF_PACKET)) {
                auto* lSocketAddress = reinterpret_cast<sockaddr_ll*>(lInterfaceAddress->ifa_addr);
                memcpy(&lReturn, &lSocketAddress->sll_addr, Net_8023_Constants::cSourceAddressLength);
                lReturn = SwapMacEndian(lReturn);
            }
#else
            if ((lInterfaceAddress->ifa_name == mAdapterName) && (lInterfaceAddress->ifa_addr->sa_family == AF_LINK)) {
                unsigned char* lAddress = reinterpret_cast<unsigned char*>(
                    LLADDR(reinterpret_cast<sockaddr_dl*>(lInterfaceAddress->ifa_addr)));
                memcpy(&lReturn, lAddress, Net_8023_Constants::cSourceAddressLength);
                lReturn = SwapMacEndian(lReturn);
            }
#endif
        }
        freeifaddrs(lInterfaceAddresses);
    } else {
        Logger::GetInstance().Log("Could not get network addresses", Logger::Level::ERROR);
    }
    return lReturn;
}
