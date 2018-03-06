#pragma once

class TTF2SDK
{
private:
    std::unique_ptr<Console> m_console;
    std::unique_ptr<Logger> m_logger;
public:
    TTF2SDK(std::unique_ptr<Console> console, std::unique_ptr<Logger> logger);
    spdlog::logger& Logger();
};

TTF2SDK& SDK();
bool SetupSDK();
