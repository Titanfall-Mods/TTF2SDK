#include "stdafx.h"

std::unique_ptr<Console> g_console;
std::unique_ptr<TTF2SDK> g_SDK;

typedef void(*SQPRINTFUNCTION)(void*, const char *, ...);

HookedFunc<void, double> _Host_RunFrame("engine.dll", "\x48\x8B\xC4\x48\x89\x58\x00\xF3\x0F\x11\x48\x00\xF2\x0F\x11\x40\x00", "xxxxxx?xxxx?xxxx?");
HookedFunc<SQInteger, void*> basePrint("client.dll", "\x40\x53\x48\x83\xEC\x30\xBA\x00\x00\x00\x00\x48\x8B\xD9", "xxxxxxx????xxx");

void newPrintFunc(void* v, const char* s, ...)
{
    va_list vl;
    va_start(vl, s);
    vprintf(s, vl);
    va_end(vl);
}


template <typename T, T> struct SDKMemberWrapper;

template<typename T, typename RT, typename... Args, RT(T::*pF)(Args...)>
struct SDKMemberWrapper<RT(T::*)(Args...), pF>
{
    static RT Call(Args&&... args)
    {
        return ((*g_SDK).*pF)(std::forward<Args>(args)...);
    }

    static RT Call(Args... args)
    {
        return ((*g_SDK).*pF)(args...);
    }
};

TTF2SDK::TTF2SDK()
{
    m_logger = spdlog::get("logger");

    if (MH_Initialize() != MH_OK)
    {
        throw std::exception("Failed to initialise MinHook");
    }

    SigScanFuncRegistry::GetInstance().ResolveAll();
    _Host_RunFrame.Hook(SDKMemberWrapper<decltype(&TTF2SDK::RunFrameHook), &TTF2SDK::RunFrameHook>::Call);
    basePrint.Hook(SDKMemberWrapper<decltype(&TTF2SDK::BasePrintHook), &TTF2SDK::BasePrintHook>::Call);
}

void TTF2SDK::RunFrameHook(double time)
{
    m_logger->debug("_Host_RunFrame: {}", time);
    return _Host_RunFrame(time);
}

SQInteger TTF2SDK::BasePrintHook(void* v)
{
    SQPRINTFUNCTION* printFunc = (SQPRINTFUNCTION*)(*(uint64_t*)((uint64_t)v + 0x50) + 0x4350);
    SQPRINTFUNCTION oldPrintFunc = *printFunc;
    *printFunc = newPrintFunc;
    SQInteger res = basePrint(v);
    *printFunc = oldPrintFunc;
    return res;
}

TTF2SDK::~TTF2SDK()
{
    MH_Uninitialize();
}

TTF2SDK& SDK()
{
    return *g_SDK;
}

void SetupLogger(const char* filename)
{
    // Create sinks to file and console
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());

    // The file sink could fail so capture the error if so
    std::unique_ptr<std::string> fileError;
    try
    {
        auto fileSink = std::make_shared<spdlog::sinks::simple_file_sink_mt>(filename, true);
        fileSink->set_force_flush(true);
        sinks.push_back(fileSink);
    }
    catch (spdlog::spdlog_ex& ex)
    {
        fileError = std::make_unique<std::string>(ex.what());
    }

    // Create logger from sink
    auto logger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
    logger->set_pattern("[%T] [%l] [thread %t] %v");
    logger->set_level(spdlog::level::debug);

    if (fileError)
    {
        logger->warn("Failed to initialise file sink, log file will be unavailable ({})", *fileError);
    }

    spdlog::register_logger(logger);
}

bool SetupSDK()
{
    // Separate try catch because these are required for logging to work
    try
    {
        g_console = std::make_unique<Console>();
        SetupLogger("TTF2SDK.log");
    }
    catch (std::exception)
    {
        return false;
    }

    try
    {
        g_SDK = std::make_unique<TTF2SDK>();
        return true;
    }
    catch (std::exception& ex)
    {
        spdlog::get("logger")->critical("Failed to initialise SDK: {}", ex.what());
        return false;
    }
}
