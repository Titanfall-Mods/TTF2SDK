#include "stdafx.h"

Logging::Logging()
{
    // Create console window and redirect stdout
    AllocConsole();

    FILE* stream;
    errno_t err = freopen_s(&stream, "CONOUT$", "w", stdout);
    if (err != 0)
    {
        // TODO: throw exception
    }

    // Create sinks to file and console
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());

    // The file sink could fail so capture the error if so
    std::unique_ptr<std::string> fileError;
    try
    {
        sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_mt>("DebugLog.log", true));
    }
    catch (spdlog::spdlog_ex& ex)
    {
        fileError = std::make_unique<std::string>(ex.what());
    }

    // Create logger from sink
    auto logger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
    logger->set_pattern("[%T] [%l] [thread %t] %v");
    spdlog::register_logger(logger);

    if (fileError)
    {
        logger->warn("Failed to initialise file sink, log file will be unavailable ({})", *fileError);
    }
}

Logging::~Logging()
{
    spdlog::get("logger")->info("Dropping logger instance");
    spdlog::drop("logger");
    fclose(stdout);
    FreeConsole();
}

Logging& Logging::GetInstance()
{
    static Logging instance;
    return instance;
}
