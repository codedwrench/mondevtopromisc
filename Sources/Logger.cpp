#include "../Includes/Logger.h"

#include <chrono>
#include <ctime>
#include <experimental/source_location>
#include <iomanip>
#include <iostream>

Logger::~Logger() {
    if (mLogOutputStream.is_open()) {
        mLogOutputStream.close();
    }
}

void Logger::Init(Level aLevel, bool aLogToDisk, const std::string& aFileName = "")
{
    SetLogLevel(aLevel);
    SetLogToDisk(aLogToDisk);
    SetFileName(aFileName);
}

void Logger::SetFileName(const std::string &aFileName) {
    mFileName = aFileName;

    if (mLogOutputStream.is_open()) {
        mLogOutputStream.close();
    }

    if (mLogToDisk) {
        mLogOutputStream.open(aFileName);
        if (mLogOutputStream.fail()) {
            std::cerr << "Opening log file failed, logging may be incomplete!" << std::endl;
        }
    }
}

void Logger::SetLogLevel(Level aLevel)
{
    mLogLevel = aLevel;
}

void Logger::SetLogToDisk(bool aLoggingToDiskEnabled) {
    if (aLoggingToDiskEnabled && !mLogOutputStream.is_open() && !mFileName.empty()) {
        mLogOutputStream.open(mFileName);
        if (mLogOutputStream.fail()) {
            std::cerr << "Opening log file failed, logging may be incomplete!" << std::endl;
        }
    } else if (!aLoggingToDiskEnabled && mLogOutputStream.is_open()) {
        mLogOutputStream.close();
    }

    mLogToDisk = aLoggingToDiskEnabled;
}

void Logger::Log(const std::string& aText, Level aLevel, const std::experimental::source_location& aLocation)
{
    std::stringstream lLogEntry;

    if (aLevel >= mLogLevel) {
        auto lTime = std::chrono::system_clock::now();
        auto lTimeAsTimeT = std::chrono::system_clock::to_time_t(lTime);
        auto lTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(lTime.time_since_epoch()) % 1000;

        lLogEntry << std::put_time(std::gmtime(&lTimeAsTimeT), "%H:%M:%S:") << std::setfill('0') <<
                  std::setw(3) << lTimeMs.count() << ": " << mLogLevelTexts.at(aLevel) << ": " << aLocation.file_name()
                  << ":"
                  << aLocation.line() << ":" << aText;

        std::cout << lLogEntry.str() << std::endl;

        // Save message to log file
        if (mLogToDisk && mLogOutputStream.is_open()) {
            mLogOutputStream << lLogEntry.str() << std::endl;
        }
    }
}