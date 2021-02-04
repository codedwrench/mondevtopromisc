#include "../Includes/WifiInterfaceWindows.h"

#include <Winsock2.h>
#include <iphlpapi.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <wlanapi.h>
#include <wtypes.h>

#ifndef UNICODE
#define UNICODE
#endif

// available network flags
#define WLAN_AVAILABLE_NETWORK_CONNECTED   0x00000001  // This network is currently connected
#define WLAN_AVAILABLE_NETWORK_HAS_PROFILE 0x00000002  // There is a profile for this network
#define WLAN_AVAILABLE_NETWORK_CONSOLE_USER_PROFILE                                                                    \
    0x00000004  // The profile is the active console user's per user profile

// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

WifiInterface::WifiInterface(std::string_view aAdapter)
{
    std::string lAdapter{aAdapter};
    size_t      lIndex{lAdapter.find("NPF")};

    // Remove NPF part
    if (lIndex != std::string::npos) {
        lIndex = lAdapter.find_first_of('{', lIndex);
        if (lIndex != std::string::npos) {
            lAdapter = lAdapter.substr(lIndex);
        } else {
            lIndex = 0;
        }
    }

    // Uppercase the string
    std::transform(lAdapter.begin(), lAdapter.end(), lAdapter.begin(), ::toupper);
    mAdapterName = lAdapter;

    GUID lAdapterGUID{};

    if (CLSIDFromString(std::wstring(lAdapter.begin(), lAdapter.end()).c_str(), &lAdapterGUID) == NOERROR) {
        mGUID = lAdapterGUID;
    }
}

WifiInterface::~WifiInterface()
{
    if (mWifiHandle != nullptr) {
        WlanCloseHandle(mWifiHandle, nullptr);
    }
}

std::vector<std::string> WifiInterface::GetAdhocNetworks()
{
    // Declare and initialize variables.
    std::vector<std::string> lReturn{};
    unsigned long            lHighestClientVersion{2};
    unsigned long            lNegotiatedVersion{0};
    DWORD                    lResult{0};

    PWLAN_AVAILABLE_NETWORK_LIST lNetworkList{nullptr};
    PWLAN_AVAILABLE_NETWORK      lNetworkInformation{nullptr};

    if (mWifiHandle == nullptr) {
        lResult = WlanOpenHandle(lHighestClientVersion, nullptr, &lNegotiatedVersion, &mWifiHandle);
    } else {
        lResult = ERROR_SUCCESS;
    }

    if (lResult == ERROR_SUCCESS) {
        lResult = WlanGetAvailableNetworkList(mWifiHandle, &mGUID, 0, nullptr, &lNetworkList);
        if (lResult == ERROR_SUCCESS) {
            Logger::GetInstance().Log("Amount of Networks: " + std::to_string(lNetworkList->dwNumberOfItems),
                                      Logger::Level::TRACE);

            for (int lCount = 0; lCount < lNetworkList->dwNumberOfItems; lCount++) {
                lNetworkInformation = &lNetworkList->Network[lCount];

                if (lNetworkInformation->dot11Ssid.uSSIDLength != 0) {
                    std::string lSSID{reinterpret_cast<char*>(lNetworkInformation->dot11Ssid.ucSSID),
                                      lNetworkInformation->dot11Ssid.uSSIDLength};

                    Logger::GetInstance().Log("SSID: " + lSSID, Logger::Level::TRACE);

                    if (lNetworkInformation->dot11BssType == dot11_BSS_type_independent) {
                        Logger::GetInstance().Log("Is Ad-Hoc network!", Logger::Level::TRACE);
                        lReturn.emplace_back(lSSID);
                    }

                    Logger::GetInstance().Log(
                        "Amount of BSSIDs: " + std::to_string(lNetworkInformation->uNumberOfBssids),
                        Logger::Level::TRACE);

                    if (lNetworkInformation->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) {
                        Logger::GetInstance().Log("Connected to this network", Logger::Level::TRACE);
                    }

                    if (lNetworkInformation->dwFlags & WLAN_AVAILABLE_NETWORK_HAS_PROFILE) {
                        Logger::GetInstance().Log("Network has a profile in network manager!", Logger::Level::TRACE);
                    }
                }
            }
        } else {
            Logger::GetInstance().Log("Could not gather scan results:" + std::system_category().message(lResult),
                                      Logger::Level::ERROR);
        }
    } else {
        mWifiHandle = nullptr;
        Logger::GetInstance().Log("Could not open WlanHandle" + std::system_category().message(lResult),
                                  Logger::Level::ERROR);
    }

    if (lNetworkList != nullptr) {
        WlanFreeMemory(lNetworkList);
        lNetworkList = nullptr;
    }
    return lReturn;
}

uint64_t WifiInterface::GetAdapterMACAddress()
{
    uint64_t lReturn{0};

    ULONG lBuffer = sizeof(IP_ADAPTER_ADDRESSES);

    GetAdaptersAddresses(0, 0, nullptr, nullptr, &lBuffer);
    std::vector<uint8_t>  lBytes(lBuffer, 0);
    PIP_ADAPTER_ADDRESSES lCurrentAddresses = (IP_ADAPTER_ADDRESSES*) lBytes.data();
    DWORD                 lResult           = GetAdaptersAddresses(0, 0, nullptr, lCurrentAddresses, &lBuffer);
    if (lResult == NO_ERROR) {
        while (lCurrentAddresses != nullptr) {
            if (lCurrentAddresses->AdapterName == mAdapterName) {
                memcpy(&lReturn, lCurrentAddresses->PhysicalAddress, Net_8023_Constants::cSourceAddressLength);
                lReturn = SwapMacEndian(lReturn);
            }
            lCurrentAddresses = lCurrentAddresses->Next;
        }
    }
    return lReturn;
}
