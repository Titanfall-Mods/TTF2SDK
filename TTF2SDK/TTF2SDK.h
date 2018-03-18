#pragma once

typedef long SQInteger;

class TTF2SDK
{
private:
    std::shared_ptr<spdlog::logger> m_logger;

    std::mutex m_mutex;
    std::string m_codeToRun = "";
    HSQUIRRELVM m_v = nullptr;

public:
    TTF2SDK();
    ~TTF2SDK();

    void RunFrameHook(double time);
    SQInteger BasePrintHook(HSQUIRRELVM v);
    void PrintFunc(HSQUIRRELVM v, const SQChar* s, va_list args);

    void SetCode(const std::string& code)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_codeToRun = code;
    }

    std::string GetCode()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        std::string data = m_codeToRun;
        m_codeToRun = "";
        return data;
    }
};

TTF2SDK& SDK();
bool SetupSDK();
