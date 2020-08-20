#include <iostream>
#include <getopt.h>
#include <pcap/pcap.h>

#include <boost/program_options.hpp>

#include "Includes/Logger.h"
#include "Includes/PCapReader.h"
#include "Includes/XLinkKaiConnection.h"
#include "Includes/TapDevice.h"

namespace
{
    constexpr Logger::Level cLogLevel{Logger::DEBUG};
    constexpr char cLogFileName[]{"log.txt"};
    constexpr bool cLogToDisk{true};
}

namespace po = boost::program_options;

int main(int argc, char** argv)
{
    std::string lBssid{};
    std::string lCaptureInterface{};
    std::string lInjectionInterface{};
    int lOption;
    po::options_description lDescription("Options");
    lDescription.add_options()
            ("help,h", "Shows this help message.")
            ("bssid,b", po::value<std::string>(&lBssid)->required(), "The BSSID to listen to.")
            ("capture_interface,c", po::value<std::string>(&lCaptureInterface)->required(),
             "The interface that will be in monitor mode listening to packets.")
            ("tap_device,t", "Enable TAP device")
            ("injection_interface,i", po::value<std::string>(&lInjectionInterface),
             "If your interface does not support packet injection, another interface can be used")
            ("xlink_kai,x", "Enables the XLink Kai interface, so packets will be sent there directly");

    po::variables_map lVariableMap;
    po::store(po::command_line_parser(argc, argv).options(lDescription).run(), lVariableMap);

    if (lVariableMap.count("help") || lVariableMap.count("h")) {
        std::cout << lDescription << std::endl;
    } else {
        po::notify(lVariableMap);
//        TapDevice lDevice;
//        if (lDevice.AllocateDevice() == 0) {
//            if(lDevice.CreateDevice(lTapDevice) == 0)
//            {
//                std::cout << lDevice.GetFd() << std::endl;
//            } else {
//                std::cout << "Failed to create TUN/TAP device" << std::endl;
//            }
//        } else {
//            std::cout << "Failed to allocate TUN/TAP device" << std::endl;
//        }


//        // TODO: Should probably move this to a different class
//        char lErrorBuffer[PCAP_ERRBUF_SIZE];
//        pcap_t *lHandler = pcap_create(lCaptureInterface.c_str(), lErrorBuffer);
//        if (lHandler != NULL) {
//            if (pcap_set_rfmon(lHandler, true) == 0) {
//                std::cout << "Monitor mode enabled on: " << lCaptureInterface << std::endl;
//
//                // TODO: Figure out correct values for this
//                pcap_set_snaplen(lHandler, 2048);  // Set the snapshot length to 2048
//                pcap_set_promisc(lHandler, 1); // Turn promiscuous mode on
//                pcap_set_timeout(lHandler, 512); // Set the timeout to 512 milliseconds
//                int lStatus = pcap_activate(lHandler);
//                if (lStatus == 0)
//                {
//                    while (lContinueLooping) {
//
//                    }
//                }
//                else
//                {
//                    std::cout << pcap_statustostr(lStatus) << std::endl;
//                }
//            } else {
//                std::cerr << "pcap_set_rfmon failed: " << lErrorBuffer << std::endl;
//            }
//        } else {
//            std::cerr << "pcap_create failed: " << lErrorBuffer << std::endl;
//        }

        Logger::GetInstance().Init(cLogLevel, cLogToDisk, cLogFileName);
        //PCapReader lPCapReader;
        XLinkKaiConnection lXLinkKaiConnection;

        if (lXLinkKaiConnection.Open()) {

            lXLinkKaiConnection.Connect();
            lXLinkKaiConnection.StartReceiverThread();
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(1));

            }
            //lPCapReader.Open("/home/codedwrench/Desktop/monitor mode.cap");

            //while (lPCapReader.ReadNextPacket()) {
            //    // Do nothing
            //}
            //lPCapReader.Close();
        }
    }

    return 0;
}
