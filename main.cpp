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
    constexpr Logger::Level cLogLevel{Logger::TRACE};
    constexpr char cLogFileName[]{"log.txt"};
    constexpr bool cLogToDisk{true};

    // Indicates if the program should be running or not, used to gracefully exit the program.
    bool gRunning{true};
}

namespace po = boost::program_options;

static void SignalHandler(const boost::system::error_code& aError, int aSignalNumber)
{
    if (!aError) {
        if (aSignalNumber == SIGINT ||
            aSignalNumber == SIGTERM) {
            // Quit gracefully.
            gRunning = false;
        }
    }
}

int main(int argc, char** argv)
{
    std::string lBssid{};
    std::string lCaptureInterface{};
    std::string lInjectionInterface{};
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

        // Handle quit signals gracefully.
        boost::asio::io_service lSignalIoService{};
        boost::asio::signal_set lSignals(lSignalIoService, SIGINT, SIGTERM);
        lSignals.async_wait(&SignalHandler);
        boost::thread lThread{[lIoService = &lSignalIoService] { lIoService->run(); }};


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

            std::chrono::time_point<std::chrono::system_clock> lStartTime{std::chrono::system_clock::now()};


            // Wait 20 seconds, this is just for testing.
            while (gRunning && (std::chrono::system_clock::now() < (lStartTime + std::chrono::minutes{60}))) {
                if (lXLinkKaiConnection.IsDisconnected()) {
                    // Try reconnecting if connection has failed.
                    lXLinkKaiConnection.Close();
                    lXLinkKaiConnection.Open();
                    lXLinkKaiConnection.Connect();
                    lXLinkKaiConnection.StartReceiverThread();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            lSignalIoService.stop();
            lThread.join();
            //lPCapReader.Open("/home/codedwrench/Desktop/monitor mode.cap");

            //while (lPCapReader.ReadNextPacket()) {
            //    // Do nothing
            //}
            //lPCapReader.Close();
        }
    }

    exit(0);
}
