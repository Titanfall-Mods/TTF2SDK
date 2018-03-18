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
    SQInteger BasePrintHook(HSQUIRRELVM v);
    void PrintFunc(HSQUIRRELVM v, const SQChar* s, va_list args);
};

TTF2SDK& SDK();
bool SetupSDK();
