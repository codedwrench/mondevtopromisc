#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Logger.h
 *
 * This file contains the header for a Logger class which will be used to log things happening in the applications based
 * on different levels.
 *
 * */

#include <array>
#include <fstream>

// Does not exist in Visual Studio yet, https://github.com/microsoft/STL/pull/664
#if defined(__GNUC__) || defined(__GNUG__)
#include <experimental/source_location>
#endif

#include <sstream>
#include <string>


/**
 * Logger class, can log text to file or stdout.
 */
class Logger
{
public:
    Logger(const Logger& aLogger) = delete;
    Logger& operator=(const Logger& aLogger) = delete;

    Logger& operator=(Logger&& aLogger) = delete;
    Logger(Logger&& aLogger)            = delete;

    /**
     * Contains loglevels, used to suppress logging based on level.
     */
    enum class Level
    {
        TRACE = 0, /**< Highest level */
        DEBUG,     /**< Debug level */
        INFO,      /**< Info level */
        WARNING,   /**< Warning level */
        ERROR      /**< Lowest level */
    };

    /**
     * Contains log levels as strings, to be used for conversion.
     */
    static constexpr std::array<std::string_view, 5> cLevelTexts{"Trace", "Debug", "Info", "Warning", "Error"};

    /**
     * Gets the Logger singleton.
     * @return The Logger object.
     */
    static Logger& GetInstance()
    {
        static Logger lInstance;
        return lInstance;
    }

    /**
     * Initializes the logger singleton.
     * @param aLevel - Loglevel to set the logger to.
     * @param aLogToDisk - Whether we should log to disk or not.
     * @param aFileName - Filename to use for logging.
     */
    void Init(Level aLevel, bool aLogToDisk, const std::string& aFileName);

    /**
     * Logs given text to file.
     * @param aText - Text to be logged.
     * @param aLevel - Loglevel to use.
     * @param aLocation - Source location (keep empty).
     */
#if defined(__GNUC__) || defined(__GNUG__)
    void Log(const std::string&                        aText,
             Level                                     aLevel,
             const std::experimental::source_location& aLocation = std::experimental::source_location::current());
#else
    void Log(const std::string& aText, Level aLevel);
#endif
    /**
     * Gets the loglevel
     */
    Level GetLogLevel();

    /**
     * Converts the loglevel to string.
     * @param aLogLevel - Log level to convert.
     * @return string with log level.
     */
    static std::string ConvertLogLevelToString(Logger::Level aLogLevel);

    /**
     * Sets the loglevel to debug, warning or error, based upon this variable certain logmessages will be shown or not
     * shown.
     * @param aLevel - The level to set.
     */
    void SetLogLevel(Level aLevel);

    /**
     * Converts the loglevel string to actual loglevel.
     * @param aLevel - The level to convert.
     * @return Level containing the loglevel. Default loglevel if string not recognized.
     */
    static Level ConvertLogLevelStringToLevel(std::string_view aLevel);

    /**
     * Sets the filename to log to when logging to disk is enabled.
     * @param aFileName - Filename to save the log to.
     */
    void SetFileName(const std::string& aFileName);

    /**
     * Enables or disables logging to disk.
     * @param aLoggingToDiskEnabled - Set true when logging to disk should be enabled.
     */
    void SetLogToDisk(bool aLoggingToDiskEnabled);

    /**
     * Enables or disables logging to screen. When using the TUI interface logging to screen is unwanted.
     * @param aLoggingToScreenEnabled - Set true when logging to screen should be enabled.
     */
    void SetLogToScreen(bool aLoggingToScreenEnabled);

private:
    Logger() = default;
    ~Logger();

    std::string   mFileName{"log.txt"};
    Level         mLogLevel{Logger::Level::ERROR};
    std::ofstream mLogOutputStream{};
    bool          mLogToDisk{false};
    bool          mLogToScreen{false};
};
