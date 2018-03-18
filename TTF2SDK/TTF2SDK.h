#pragma once

typedef long SQInteger;

class TTF2SDK
{
private:
    std::shared_ptr<spdlog::logger> m_logger;
public:
    TTF2SDK();
    ~TTF2SDK();

    void RunFrameHook(double time);
    SQInteger BasePrintHook(void* sqVM);
};

TTF2SDK& SDK();
bool SetupSDK();
