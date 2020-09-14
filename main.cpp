#include <iostream>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "Includes/Logger.h"
#include "Includes/WirelessMonitorDevice.h"
#include "Includes/XLinkKaiConnection.h"

namespace
{
    constexpr Logger::Level cLogLevel{Logger::DEBUG};
    constexpr char          cLogFileName[]{"log.txt"};
    constexpr bool          cLogToDisk{true};

    // Indicates if the program should be running or not, used to gracefully exit the program.
    bool gRunning{true};
}  // namespace

namespace po = boost::program_options;

static void SignalHandler(const boost::system::error_code& aError, int aSignalNumber)
{
    if (!aError) {
        if (aSignalNumber == SIGINT || aSignalNumber == SIGTERM) {
            // Quit gracefully.
            gRunning = false;
        }
    }
}

int main(int argc, char** argv)
{
    std::string             lBssid{};
    std::string             lCaptureInterface{};
    std::string             lInjectionInterface{};
    po::options_description lDescription("Options");
    // clang-format off
    lDescription.add_options()
            ("help,h", "Shows this help message.")
            ("bssid,b", po::value<std::string>(&lBssid)->required(), "The BSSID to listen to.")
            ("capture_interface,c", po::value<std::string>(&lCaptureInterface)->required(),
             "The interface that will be in monitor mode listening to packets.")
            ("injection_interface,i", po::value<std::string>(&lInjectionInterface),
             "If your interface does not support packet injection, another interface can be used")
            ("xlink_kai,x", "Enables the XLink Kai interface, so packets will be sent there directly");
    // clang-format on
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
        std::shared_ptr<WirelessMonitorDevice> lMonitorDevice = std::make_shared<WirelessMonitorDevice>();
        std::shared_ptr<ISendReceiveDevice>    lOutputDevice  = nullptr;

        if (lVariableMap.count("xlink_kai")) {
            std::shared_ptr<XLinkKaiConnection> lXLinkKai = std::make_shared<XLinkKaiConnection>();
            if (lXLinkKai->Open(XLinkKai_Constants::cIp)) {
                lXLinkKai->SetSendReceiveDevice(lMonitorDevice);
                if (lXLinkKai->StartReceiverThread()) {
                    lOutputDevice = lXLinkKai;
                } else {
                    lXLinkKai->Close();
                }
            } else {
                Logger::GetInstance().Log("Opening of XLink Kai device failed!", Logger::ERR);
            }
        }

        if (lOutputDevice != nullptr) {
            if (lMonitorDevice->Open(lCaptureInterface)) {
                lMonitorDevice->SetBSSID(lBssid);
                lMonitorDevice->SetSendReceiveDevice(lOutputDevice);
                if (lMonitorDevice->StartReceiverThread()) {
                    std::chrono::time_point<std::chrono::system_clock> lStartTime{std::chrono::system_clock::now()};

                    // Wait 60 seconds, this is just for testing.
                    while (gRunning && (std::chrono::system_clock::now() < (lStartTime + std::chrono::minutes{60}))) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }

                lOutputDevice->Close();
                lMonitorDevice->Close();
            } else {
                std::cerr << "Could not open capture interface" << std::endl;
            }
        } else {
            std::cerr << "Output device not specified!" << std::endl << "Supported options: --xlink_kai" << std::endl;
        }

        lSignalIoService.stop();
        lThread.join();
    }

    exit(0);
}
