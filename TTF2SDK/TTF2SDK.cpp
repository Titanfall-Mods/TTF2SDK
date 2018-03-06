#include "stdafx.h"

TTF2SDK::TTF2SDK(std::unique_ptr<Console> console, std::unique_ptr<::Logger> logger)
    : m_console(std::move(console)), m_logger(std::move(logger))
{
    
}

spdlog::logger& TTF2SDK::Logger()
{
    return m_logger->Get();
}

std::unique_ptr<TTF2SDK> g_SDK;

TTF2SDK& SDK()
{
    return *g_SDK;
}

bool SetupSDK()
{
    std::unique_ptr<Console> console;
    std::unique_ptr<Logger> logger;

    // Separate try catch because these are required for logging to work
    try
    {
        console = std::make_unique<Console>();
        logger = std::make_unique<Logger>("TTF2SDK.log");
    }
    catch (std::exception)
    {
        return false;
    }

    try
    {
        g_SDK = std::make_unique<TTF2SDK>(std::move(console), std::move(logger));
        return true;
    }
    catch (std::exception& ex)
    {
        logger->Get().critical("Failed to initialise SDK: {}", ex.what());
        return false;
    }
}