#pragma once

class Logger
{
private:
    std::unique_ptr<spdlog::logger> m_logger;

public:
    Logger(const char* filename);
    ~Logger();
    spdlog::logger& Get();
};
