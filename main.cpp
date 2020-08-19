#include <iostream>
#include <getopt.h>
#include <pcap/pcap.h>

#include "Includes/Logger.h"
#include "Includes/PCapReader.h"
#include "Includes/TapDevice.h"

int main(int argc, char **argv) {

    bool lHelpFlag = false;
    std::string lBssid{};
    std::string lCaptureInterface{};
    std::string lInjectionInterface{};
    std::string lTapDevice{};

    int lOption;

    while ((lOption = getopt(argc, argv, ":hb:c:d:i:")) != -1) {
        switch (lOption) {
            case 'h':
                lHelpFlag = true;
                break;
            case 'b':
                lBssid = optarg;
                break;
            case 'c':
                lCaptureInterface = optarg;
                break;
            case 'd':
                lTapDevice = optarg;
                break;
            case ':':
                std::cerr << "Option -" << static_cast<char>(optopt) << " requires an argument." << std::endl;
                lHelpFlag = true;
                break;
            case '?':
                std::cerr << "Unknown option - " << static_cast<char>(optopt) << std::endl;
                lHelpFlag = true;
                break;
            default:
                lHelpFlag = true;
                std::cerr << "Unknown error occurred" << std::endl;
                break;
        }
    }

    if (argc < 2) {
        std::cerr << "No options provided, -b and -c are least required!" << std::endl;
        lHelpFlag = true;
    } else if ((!lHelpFlag) && (lBssid.empty() || lCaptureInterface.empty())) {
        std::cerr << "Not enough options provided, -b and -c are least required!" << std::endl;
        lHelpFlag = true;
    }

    if (lHelpFlag) {
        std::cout << "Usage example: " << argv[0] << " -c wlp4s0" << std::endl <<
                  "Options: -h:    Shows this help text" << std::endl <<
                  "         -b %s: REQUIRED: The BSSID to listen to" << std::endl <<
                  "         -c %s: REQUIRED: The capture interface to use" << std::endl <<
                  "         -i %s: The injection interface to use, " << std::endl <<
                  "                keep empty if packet injection in monitor mode is to be used" << std::endl <<
                  "         -d %s: What to name the tap device, keep empty if you want it to be named automatically"
                  << std::endl;
    } else {
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

        Logger::GetInstance().Init(Logger::DEBUG, true, "log.txt");
        PCapReader lPCapReader;
        lPCapReader.Open("/home/codedwrench/Desktop/monitor mode.cap");

        while (lPCapReader.ReadNextPacket()) {
            // Do nothing
        }
        lPCapReader.Close();
    }

    return 0;
}
