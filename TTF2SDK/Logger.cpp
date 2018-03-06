#include "stdafx.h"

Logger::Logger(const char* filename)
{
    // Create sinks to file and console
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());

    // The file sink could fail so capture the error if so
    std::unique_ptr<std::string> fileError;
    try
    {
        sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_mt>(filename, true));
    }
    catch (spdlog::spdlog_ex& ex)
    {
        fileError = std::make_unique<std::string>(ex.what());
    }

    // Create logger from sink
    m_logger = std::make_unique<spdlog::logger>("logger", begin(sinks), end(sinks));
    m_logger->set_pattern("[%T] [%l] [thread %t] %v");

    if (fileError)
    {
        m_logger->warn("Failed to initialise file sink, log file will be unavailable ({})", *fileError);
    }
}

Logger::~Logger()
{
    
}

spdlog::logger& Logger::Get()
{
    return *m_logger;
}

