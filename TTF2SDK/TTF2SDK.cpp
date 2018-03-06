#include "stdafx.h"

TTF2SDK::TTF2SDK(std::unique_ptr<Console> console, std::unique_ptr<::Logger> logger)
    : m_console(std::move(console)), m_logger(std::move(logger))
{
    
}

spdlog::logger& TTF2SDK::Logger()
{
    return m_logger->Get();
}
