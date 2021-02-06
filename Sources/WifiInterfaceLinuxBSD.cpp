#include "../Includes/WifiInterfaceLinuxBSD.h"

#include <cerrno>

#include <ifaddrs.h>
#include <net/if.h>

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"

#ifdef __linux__
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#else
#include <net/if_dl.h>
#endif

using namespace WifiInterface_Constants;

WifiInterface::WifiInterface(std::string_view aAdapter)
{
    mAdapterName = aAdapter;
    SetBSSPolicy();
    // Open socket to kernel.
    mSocket = nl_socket_alloc();  // Allocate new netlink socket in memory
    genl_connect(mSocket);        // Create file descriptor and bind socket
    mDriverId = genl_ctrl_resolve(mSocket, WifiInterface_Constants::cDriverName.data());  // Find the nl80211 driver ID
    mNetworkAdapterIndex = if_nametoindex(mAdapterName.data());  // Use this wireless interface for scanning
}

WifiInterface::~WifiInterface() {}

void WifiInterface::SetBSSPolicy()
{
    mBSSPolicy[NL80211_BSS_TSF]                  = {NLA_U64, 0, 0};
    mBSSPolicy[NL80211_BSS_FREQUENCY]            = {NLA_U32, 0, 0};
    mBSSPolicy[NL80211_BSS_BSSID]                = {NLA_UNSPEC, 0, 0};
    mBSSPolicy[NL80211_BSS_BEACON_INTERVAL]      = {NLA_U16, 0, 0};
    mBSSPolicy[NL80211_BSS_CAPABILITY]           = {NLA_U16, 0, 0};
    mBSSPolicy[NL80211_BSS_INFORMATION_ELEMENTS] = {NLA_UNSPEC, 0, 0};
    mBSSPolicy[NL80211_BSS_SIGNAL_MBM]           = {NLA_U32, 0, 0};
    mBSSPolicy[NL80211_BSS_SIGNAL_UNSPEC]        = {NLA_U8, 0, 0};
    mBSSPolicy[NL80211_BSS_STATUS]               = {NLA_U32, 0, 0};
    mBSSPolicy[NL80211_BSS_SEEN_MS_AGO]          = {NLA_U32, 0, 0};
    mBSSPolicy[NL80211_BSS_BEACON_IES]           = {NLA_UNSPEC, 0, 0};
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
            }
#else
            if ((lInterfaceAddress->ifa_name == mAdapterName) && (lInterfaceAddress->ifa_addr->sa_family == AF_LINK)) {
                unsigned char* lAddress = reinterpret_cast<unsigned char*>(
                    LLADDR(reinterpret_cast<sockaddr_dl*>(lInterfaceAddress->ifa_addr)));
                memcpy(&lReturn, lAddress, Net_8023_Constants::cSourceAddressLength);
            }
#endif
        }
        freeifaddrs(lInterfaceAddresses);
    } else {
        Logger::GetInstance().Log("Could not get network addresses", Logger::Level::ERROR);
    }
    return lReturn;
}

static int ErrorHandler(sockaddr_nl* /*aSockAddress*/, nlmsgerr* aError, void* aArgument)
{
    // Callback for errors.
    int* lReturn = static_cast<int*>(aArgument);
    *lReturn     = aError->error;
    return NL_STOP;
}


static int FinishHandler(nl_msg* aMessage, void* aArgument)
{
    // Callback for NL_CB_FINISH.
    int* lReturn = static_cast<int*>(aArgument);
    *lReturn     = 0;
    return NL_SKIP;
}


static int AcknowledgeHandler(nl_msg* aMessage, void* aArgument)
{
    // Callback for NL_CB_ACK.
    int* lReturn = static_cast<int*>(aArgument);
    *lReturn     = 0;
    return NL_STOP;
}


static int SkipSequenceCheck(nl_msg* aMessage, void* aArgument)
{
    // Callback for NL_CB_SEQ_CHECK.
    return NL_OK;
}


static int FamilyHandler(nl_msg* aMessage, void* aArgument)
{
    // https://git.kernel.org/pub/scm/linux/kernel/git/jberg/iw.git/tree/genl.c used as reference.
    auto*                                     lGroup{reinterpret_cast<HandlerArguments*>(aArgument)};
    std::array<nlattr*, NL80211_ATTR_MAX + 1> lIndices{};
    auto*   lGenlMessageHeader{reinterpret_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(aMessage)))};
    nlattr* lMultiCastGroup{};
    int     lRemaining{};

    nla_parse(lIndices.data(),
              CTRL_ATTR_MAX,
              genlmsg_attrdata(lGenlMessageHeader, 0),
              genlmsg_attrlen(lGenlMessageHeader, 0),
              nullptr);

    if (lIndices.at(CTRL_ATTR_MCAST_GROUPS) != nullptr) {
        // Loop through the multicast groups
        nla_for_each_nested(lMultiCastGroup, lIndices.at(CTRL_ATTR_MCAST_GROUPS), lRemaining)
        {
            std::array<nlattr*, CTRL_ATTR_MCAST_GRP_MAX + 1> lMulticastGroupIndices;

            nla_parse(lMulticastGroupIndices.data(),
                      CTRL_ATTR_MCAST_GRP_MAX,
                      reinterpret_cast<nlattr*>(nla_data(lMultiCastGroup)),
                      nla_len(lMultiCastGroup),
                      nullptr);

            // If the group exists in this index
            if ((lMulticastGroupIndices.at(CTRL_ATTR_MCAST_GRP_NAME) != nullptr) &&
                (lMulticastGroupIndices.at(CTRL_ATTR_MCAST_GRP_ID) != nullptr)) {
                // If the group matches the expectation
                std::string lCurrentGroup{
                    reinterpret_cast<char*>(nla_data(lMulticastGroupIndices.at(CTRL_ATTR_MCAST_GRP_NAME))),
                    static_cast<size_t>(nla_len(lMulticastGroupIndices.at(CTRL_ATTR_MCAST_GRP_NAME)) - 1)};

                if (lCurrentGroup == lGroup->group) {
                    lGroup->id = nla_get_u32(lMulticastGroupIndices.at(CTRL_ATTR_MCAST_GRP_ID));
                    // No other solution than calling break here sadly
                    break;
                }
            }
        }
    }
    return NL_SKIP;
}

int WifiInterface::GetMulticastId()
{
    //  https://git.kernel.org/pub/scm/linux/kernel/git/jberg/iw.git/tree/genl.c used as reference.
    nl_msg* lMessage{nlmsg_alloc()};
    nl_cb*  lCallback{nl_cb_alloc(NL_CB_DEFAULT)};
    int     lReturn{0};

    HandlerArguments lGroup{cScanCommand.data(), -ENOENT};

    if (lMessage != nullptr && lCallback != nullptr) {
        int lControlId = genl_ctrl_resolve(mSocket, cControlCommand.data());
        // Get the command
        genlmsg_put(lMessage, 0, 0, lControlId, 0, 0, CTRL_CMD_GETFAMILY, 0);
        if (nla_put_string(lMessage, CTRL_ATTR_FAMILY_NAME, cDriverName.data()) >= 0) {
            lReturn = nl_send_auto_complete(mSocket, lMessage);
            if (lReturn >= 0) {
                // Set handlers
                nl_cb_err(lCallback, NL_CB_CUSTOM, ErrorHandler, &lReturn);
                nl_cb_set(lCallback, NL_CB_ACK, NL_CB_CUSTOM, AcknowledgeHandler, &lReturn);
                nl_cb_set(lCallback, NL_CB_VALID, NL_CB_CUSTOM, FamilyHandler, &lGroup);

                while (lReturn > 0) {
                    // Receive messages until lReturn set to 0 by one of the callbacks
                    nl_recvmsgs(mSocket, lCallback);
                }

                if (lReturn == 0) {
                    lReturn = lGroup.id;
                }
            }
        }
    } else {
        lReturn = -ENOMEM;
    }

    if (lMessage != nullptr) {
        nlmsg_free(lMessage);
    }
    if (lCallback != nullptr) {
        nl_cb_put(lCallback);
    }

    return lReturn;
}


/**
 * Grabs the SSID from the Information Elements given back by DumpResults.
 * @param aBeaconInformation - The IE-fields.
 * @param aBeaconLength - The length of the IE-fields.
 * @return The SSID found.
 */
static std::string GetSSIDFromIE(unsigned char* aBeaconInformation, int aBeaconLength)
{
    std::string lSSID{};
    uint8_t     lLength{};
    uint8_t*    lData{};

    while (aBeaconLength >= 2 && aBeaconLength >= aBeaconInformation[1]) {
        // Index 1 contains SSID Length
        int lSSIDLength{aBeaconInformation[1]};
        if (aBeaconInformation[0] == 0 && lSSIDLength >= 0 && lSSIDLength <= 32) {
            // At index 2 the actual SSID begins
            lData = aBeaconInformation + 2;
            lSSID.append(std::string(reinterpret_cast<char*>(lData), lSSIDLength));
            break;
        }
        // Remove size of SSID + header from total length
        aBeaconLength -= aBeaconInformation[1] + 2;
        // Proceed pointer to next SSID in list
        aBeaconInformation += aBeaconInformation[1] + 2;
    }
    return lSSID;
}

/**
 * Handles a callback from the kernel, sets a results struct which is used in another part of the code.
 * @param aMessage - Filled in by the kernel.
 * @param aArgument - TriggerResults struct that needs to be filled in.
 * @return NL_SKIP
 */
static int CallbackTrigger(nl_msg* aMessage, void* aArgument)
{
    // Called by the kernel when the scan is done or has been aborted.
    auto* lGenlMessageHeader = reinterpret_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(aMessage)));
    auto* lResults           = reinterpret_cast<TriggerResults*>(aArgument);

    if (lGenlMessageHeader->cmd == NL80211_CMD_SCAN_ABORTED) {
        lResults->done    = 1;
        lResults->aborted = 1;
    } else if (lGenlMessageHeader->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
        lResults->done    = 1;
        lResults->aborted = 0;
    }  // else probably an uninteresting multicast message.

    return NL_SKIP;
}

/**
 * Prints out the results and add adhocnetworks to vector.
 * @param aMessage - The message that came from the kernel where we can figure out what data there is.
 * @param aArgument - a void* BSS service info and with an array of adhocnetworks, format needs to be
 * DumpResultArgument.
 * @return NL_SKIP
 */
static int DumpResults(nl_msg* aMessage, void* aArgument)
{
    int   lReturn{};
    auto* lArgument = reinterpret_cast<DumpResultArgument*>(aArgument);

    // Called by the kernel with a dump of the successful scan's data. Called for each SSID.
    auto* lGenlMessageHeader = reinterpret_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(aMessage)));
    std::array<nlattr*, NL80211_ATTR_MAX + 1> lIndices{};
    std::array<nlattr*, NL80211_ATTR_MAX + 1> lBSS{};

    // Parse and error check.
    nla_parse(lIndices.data(),
              NL80211_ATTR_MAX,
              genlmsg_attrdata(lGenlMessageHeader, 0),
              genlmsg_attrlen(lGenlMessageHeader, 0),
              nullptr);
    if (lIndices.at(NL80211_ATTR_BSS) != nullptr) {
        if (nla_parse_nested(
                lBSS.data(), NL80211_BSS_MAX, lIndices.at(NL80211_ATTR_BSS), lArgument->bssserviceinfo.data()) == 0) {
            if (lBSS.at(NL80211_BSS_BSSID) != nullptr && lBSS.at(NL80211_BSS_BEACON_IES) != nullptr) {
                // Grab the SSID and print it
                std::string lSSID{GetSSIDFromIE(reinterpret_cast<unsigned char*>(lBSS.at(NL80211_BSS_BEACON_IES)),
                                                nla_len(lBSS.at(NL80211_BSS_BEACON_IES)))};

                // I'm also filtering the hidden SSIDs, (" ")
                if (!lSSID.empty() && lSSID != " ") {
                    Logger::GetInstance().Log(lSSID, Logger::Level::TRACE);

                    IWifiInterface::WifiInformation lInformation{};

                    // Grab the connected information
                    if (lBSS.at(NL80211_BSS_STATUS) != nullptr) {
                        uint32_t lBSSStatus{nla_get_u32(lBSS.at(NL80211_BSS_STATUS))};
                        lInformation.isconnected =
                            lBSSStatus == NL80211_BSS_STATUS_ASSOCIATED || lBSSStatus == NL80211_BSS_STATUS_IBSS_JOINED;
                    }

                    // Grab the SSID
                    lInformation.ssid = lSSID;

                    // Grab the BSSID
                    memcpy(lInformation.bssid.data(), nla_data(lBSS.at(NL80211_BSS_BSSID)), 6);

                    // Grab the frequency
                    if (lBSS.at(NL80211_BSS_FREQUENCY) != nullptr) {
                        lInformation.frequency = nla_get_u32(lBSS.at(NL80211_BSS_FREQUENCY));
                    }

                    // Using the Capability field, index 0, field ESS, if this is 0, the network is an adhoc
                    // network.
                    if ((nla_get_u32(lBSS.at(NL80211_BSS_CAPABILITY)) & 0b1U) == 0U) {
                        Logger::GetInstance().Log("Is Ad-Hoc network!", Logger::Level::TRACE);
                        lInformation.isadhoc = true;
                    }

                    // Add the network to the vector
                    lArgument->adhocnetworks.emplace_back(lInformation);

                    // Associated info will always be put at the beginning
                    if (lInformation.isconnected) {
                        std::swap(lArgument->adhocnetworks.back(), lArgument->adhocnetworks.front());
                    }
                }
            }
        } else {
            Logger::GetInstance().Log("Parsing info failed!", Logger::Level::ERROR);
        }
    } else {
        Logger::GetInstance().Log("Basic service set info missing!", Logger::Level::ERROR);
    }

    return NL_SKIP;
}

bool WifiInterface::ScanTrigger()
{
    // Starts the scan and waits for it to finish. Does not return until the scan is done or has been aborted.
    TriggerResults lResults{};
    int            lError{};
    int            lSuccess{};
    bool           lReturn{};
    int            lMulticastId = GetMulticastId();

    nl_socket_add_membership(mSocket, lMulticastId);  // Without this, CallbackTrigger() won't be called.

    // Allocate the messages and callback handler.
    nl_msg* lMessage{nlmsg_alloc()};
    if (lMessage != nullptr) {
        nl_msg* lSSIDsToScan = nlmsg_alloc();
        if (lSSIDsToScan != nullptr) {
            nl_cb* lCallback{nl_cb_alloc(NL_CB_DEFAULT)};
            if (lCallback != nullptr) {
                // Setup the messages and callback handler.
                genlmsg_put(
                    lMessage, 0, 0, mDriverId, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);  // Setup which command to run.

                nla_put_u32(lMessage,
                            NL80211_ATTR_IFINDEX,
                            mNetworkAdapterIndex);  // Add message attribute, which interface to use.

                nla_put(lSSIDsToScan, 1, 0, "");  // Scan all SSIDs.

                nla_put_nested(lMessage,
                               NL80211_ATTR_SCAN_SSIDS,
                               lSSIDsToScan);  // Add message attribute, which SSIDs to scan for.

                nlmsg_free(lSSIDsToScan);  // Copied to `Message` above, no longer need this.

                // Add callbacks
                nl_cb_set(lCallback, NL_CB_VALID, NL_CB_CUSTOM, CallbackTrigger, &lResults);
                nl_cb_err(lCallback, NL_CB_CUSTOM, ErrorHandler, &lError);
                nl_cb_set(lCallback, NL_CB_FINISH, NL_CB_CUSTOM, FinishHandler, &lError);
                nl_cb_set(lCallback, NL_CB_ACK, NL_CB_CUSTOM, AcknowledgeHandler, &lError);
                nl_cb_set(lCallback,
                          NL_CB_SEQ_CHECK,
                          NL_CB_CUSTOM,
                          SkipSequenceCheck,
                          nullptr);  // No sequence checking for multicast messages.


                // Send NL80211_CMD_TRIGGER_SCAN to start the scan. The kernel may reply with
                // NL80211_CMD_NEW_SCAN_RESULTS on success or NL80211_CMD_SCAN_ABORTED if another scan was started
                // by another process.
                lSuccess = nl_send_auto(mSocket, lMessage);  // Send the message.
                Logger::GetInstance().Log("Waiting for scan to complete...", Logger::Level::DEBUG);
                while (lError > 0) {
                    // First wait for AcknowledgeHandler(). This helps with basic errors.
                    lSuccess = nl_recvmsgs(mSocket, lCallback);
                }

                if (lError < 0) {
                    // Put resulting lErroror message in return value
                    lSuccess = lError;
                }

                if (lSuccess >= 0) {
                    lError = 1;
                    while (lResults.done == 0 && lResults.aborted == 0 && lError > 0) {
                        // Now wait until the scan is done or aborted
                        nl_recvmsgs(mSocket, lCallback);
                    }

                    if (lError < 0) {
                        Logger::GetInstance().Log(std::string("Error occured: ") + nl_geterror(-lError),
                                                  Logger::Level::ERROR);
                    }

                    if (lResults.aborted != 0) {
                        Logger::GetInstance().Log("Kernel aborted scan", Logger::Level::WARNING);
                    }

                    if (lCallback != nullptr) {
                        nl_cb_put(lCallback);
                    }
                } else {
                    Logger::GetInstance().Log(std::string("nl_recvmsgs() returned") + nl_geterror(-lSuccess),
                                              Logger::Level::ERROR);
                }
            } else {
                Logger::GetInstance().Log("Failed to allocate netlink callbacks", Logger::Level::ERROR);
                nlmsg_free(lSSIDsToScan);
            }
        } else {
            Logger::GetInstance().Log("Failed to allocate netlink message for SSIDsToScan", Logger::Level::ERROR);
        }
    } else {
        Logger::GetInstance().Log("Failed to allocate netlink message for message", Logger::Level::ERROR);
    }

    if (lMessage != nullptr) {
        nlmsg_free(lMessage);
    }

    nl_socket_drop_membership(mSocket, lMulticastId);  // No longer need this.
    return lReturn;
}


std::vector<IWifiInterface::WifiInformation>& WifiInterface::GetAdhocNetworks()
{
    // Issue NL80211_CMD_TRIGGER_SCAN to the kernel and wait for it to finish.
    bool lSuccess{ScanTrigger()};
    if (lSuccess) {
        // Now get info for all SSIDs detected.
        nl_msg* lMessage = nlmsg_alloc();  // Allocate a message.

        // We want to dump all the information
        genlmsg_put(lMessage, 0, 0, mDriverId, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);

        // Add message attribute, which interface to use
        nla_put_u32(lMessage, NL80211_ATTR_IFINDEX, mNetworkAdapterIndex);

        // Clear previous scan info
        mLastReceivedScanInformation.clear();

        // Add the callback
        DumpResultArgument lArgument{mBSSPolicy, mLastReceivedScanInformation};
        nl_socket_modify_cb(mSocket, NL_CB_VALID, NL_CB_CUSTOM, DumpResults, &lArgument);

        // Send the message
        int lError{nl_send_auto(mSocket, lMessage)};
        if (lError >= 0) {
            // Retrieve the kernel's answer. DumpResults() prints SSIDs.
            lError = nl_recvmsgs_default(mSocket);
            if (lError < 0) {
                Logger::GetInstance().Log("Failed to nl_recvmsgs_default " + std::to_string(lError),
                                          Logger::Level::ERROR);
            }
        } else {
            Logger::GetInstance().Log("Failed to send message " + std::to_string(lError), Logger::Level::ERROR);
        }

        nlmsg_free(lMessage);
    } else {
        nl_socket_free(mSocket);
        mSocket              = nullptr;
        mDriverId            = 0;
        mNetworkAdapterIndex = 0;
    }

    return mLastReceivedScanInformation;
}

bool WifiInterface::SetIBSSType()
{
    bool lReturn{};

    // Allocate the messages and callback handler.
    nl_msg* lMessage{nlmsg_alloc()};
    if (lMessage != nullptr) {
        genlmsg_put(lMessage,
                    0,
                    0,
                    mDriverId,
                    0,
                    (NLM_F_REQUEST | NLM_F_ACK),
                    NL80211_CMD_SET_INTERFACE,
                    0);  // Setup which
                         // command to run.
        // Interface
        nla_put_u32(lMessage, NL80211_ATTR_IFINDEX, mNetworkAdapterIndex);
        // What type to set
        nla_put_u32(lMessage, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_ADHOC);


        int lError{1};
        // Send the message.
        lError = nl_send_auto_complete(mSocket, lMessage);
        Logger::GetInstance().Log("Setting BSS type", Logger::Level::TRACE);

        if (lError >= 0) {
            // When this is done command is successful
            lError = nl_recvmsgs_default(mSocket);
            if (lError >= 0) {
                lReturn = true;
            } else {
                Logger::GetInstance().Log(std::string("Failed to nl_recvmsgs_default ") + nl_geterror(-lError),
                                          Logger::Level::ERROR);
            }
        } else {
            Logger::GetInstance().Log(std::string("Failed to nl_send_auto_complete ") + nl_geterror(-lError),
                                      Logger::Level::ERROR);
        }
    } else {
        Logger::GetInstance().Log("Failed to allocate netlink message for message", Logger::Level::ERROR);
    }

    // Cleanup.
    if (lMessage != nullptr) {
        nlmsg_free(lMessage);
    }

    return lReturn;
}

bool WifiInterface::Connect(const IWifiInterface::WifiInformation& aConnection)
{
    bool lReturn{};

    // Disconnect from the old network
    if (mLastReceivedScanInformation.begin()->isconnected) {
        if (mLastReceivedScanInformation.begin()->isadhoc) {
            LeaveIBSS();
        } else {
            SetIBSSType();
        }
    } else {
        // If not connected just set IBSS mode anyway and leave old IBSS for good measure, even if there aren't any
        SetIBSSType();
        LeaveIBSS();
    }

    // Allocate the messages and callback handler.
    nl_msg* lMessage{nlmsg_alloc()};
    if (lMessage != nullptr) {
        // Set connect command
        genlmsg_put(lMessage, 0, 0, mDriverId, 0, (NLM_F_REQUEST | NLM_F_ACK), NL80211_CMD_JOIN_IBSS, 0);

        // Interface, what ssid to connect to, what bssid to connect to and frequency
        nla_put_u32(
            lMessage, NL80211_ATTR_IFINDEX, mNetworkAdapterIndex);  // Add message attribute, which interface to use.
        nla_put(lMessage, NL80211_ATTR_SSID, aConnection.ssid.length(), aConnection.ssid.data());
        nla_put_u32(lMessage,
                    NL80211_ATTR_WIPHY_FREQ,
                    aConnection.frequency);  // Add message attribute, which frequency to use.
        nla_put(lMessage, NL80211_ATTR_MAC, 6, aConnection.bssid.data());
        Logger::GetInstance().Log("Connecting to:" + aConnection.ssid, Logger::Level::DEBUG);

        int lError{1};
        // Send the message.
        lError = nl_send_auto_complete(mSocket, lMessage);
        Logger::GetInstance().Log("Leaving AdHoc network", Logger::Level::TRACE);

        if (lError >= 0) {
            // When this is done command is successful
            lError = nl_recvmsgs_default(mSocket);
            if (lError >= 0) {
                lReturn = true;
            } else {
                Logger::GetInstance().Log(std::string("Failed to nl_recvmsgs_default ") + nl_geterror(-lError),
                                          Logger::Level::ERROR);
            }
        } else {
            Logger::GetInstance().Log(std::string("Failed to nl_send_auto_complete ") + nl_geterror(-lError),
                                      Logger::Level::ERROR);
        }
    } else {
        Logger::GetInstance().Log("Failed to allocate netlink message for message", Logger::Level::ERROR);
    }

    Logger::GetInstance().Log("Connection is done", Logger::Level::TRACE);

    // Cleanup.
    if (lMessage != nullptr) {
        nlmsg_free(lMessage);
    }

    return lReturn;
}

bool WifiInterface::LeaveIBSS()
{
    bool lReturn{};

    // Allocate the messages and callback handler.
    nl_msg* lMessage{nlmsg_alloc()};
    if (lMessage != nullptr) {
        // Set leave BSS command
        genlmsg_put(lMessage, 0, 0, mDriverId, 0, (NLM_F_REQUEST | NLM_F_ACK), NL80211_CMD_LEAVE_IBSS, 0);

        // Interface to use.
        nla_put_u32(lMessage, NL80211_ATTR_IFINDEX, mNetworkAdapterIndex);

        int lError{1};
        // Send the message.
        lError = nl_send_auto_complete(mSocket, lMessage);
        Logger::GetInstance().Log("Leaving AdHoc network", Logger::Level::TRACE);

        if (lError >= 0) {
            // When this is done command is successful
            lError = nl_recvmsgs_default(mSocket);
            if (lError >= 0) {
                lReturn = true;
            } else {
                Logger::GetInstance().Log(std::string("Failed to nl_recvmsgs_default ") + nl_geterror(-lError),
                                          Logger::Level::ERROR);
            }
        } else {
            Logger::GetInstance().Log(std::string("Failed to nl_send_auto_complete ") + nl_geterror(-lError),
                                      Logger::Level::ERROR);
        }
    } else {
        Logger::GetInstance().Log("Failed to allocate netlink message for message", Logger::Level::ERROR);
    }

    // Cleanup
    if (lMessage != nullptr) {
        nlmsg_free(lMessage);
    }

    return lReturn;
}
