#include <iostream>
#include <pcap/pcap.h>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "Includes/Logger.h"
#include "Includes/PCapReader.h"
#include "Includes/WirelessCaptureDevice.h"
#include "Includes/XLinkKaiConnection.h"

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


        Logger::GetInstance().Init(cLogLevel, cLogToDisk, cLogFileName);
        WirelessCaptureDevice lCaptureDevice;
        XLinkKaiConnection lXLinkKaiConnection;
        PacketConverter lPacketConverter{true};

        if (lCaptureDevice.Open(lCaptureInterface)) {
            if (lXLinkKaiConnection.Open()) {
                lCaptureDevice.SetBSSIDFilter(lBssid);

                lXLinkKaiConnection.Connect();
                lXLinkKaiConnection.StartReceiverThread();

                std::chrono::time_point<std::chrono::system_clock> lStartTime{
                        std::chrono::system_clock::now()};

                // Wait 60 seconds, this is just for testing.
                while (gRunning &&
                       (std::chrono::system_clock::now() < (lStartTime + std::chrono::minutes{60}))) {
                    if (lXLinkKaiConnection.IsDisconnected() &&
                        (!lXLinkKaiConnection.IsConnecting())) {
                        // Try reconnecting if connection has failed.
                        lXLinkKaiConnection.Close();
                        lXLinkKaiConnection.Open();
                        lXLinkKaiConnection.Connect();
                        lXLinkKaiConnection.StartReceiverThread();
                    }

                    //TODO: Needs to be a nicer send function in WirelessCaptureDevice
                    if (lCaptureDevice.ReadNextPacket()) {
                        std::string lData = lPacketConverter.ConvertPacketToPromiscuous(lCaptureDevice.DataToString());
                        if (!lData.empty()) {
                            lData.insert(0, XLinkKai_Constants::cEthernetDataString);
                            lXLinkKaiConnection.Send(lData);
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                lSignalIoService.stop();
                lThread.join();
                lCaptureDevice.Close();
            }
        } else {
            std::cerr << "could not open capture interface" << std::endl;
        }
    }

    exit(0);
}
