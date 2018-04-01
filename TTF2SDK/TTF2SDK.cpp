#include "stdafx.h"

std::unique_ptr<Console> g_console;
std::unique_ptr<TTF2SDK> g_SDK;

struct CompileBufferState
{
    const SQChar* buffer;
    const SQChar* bufferPlusLength;
    const SQChar* bufferAgain;

    CompileBufferState(const std::string& code)
    {
        buffer = code.c_str();
        bufferPlusLength = code.c_str() + code.size();
        bufferAgain = code.c_str();
    }
};

/*
[12:13 AM] Wilko: so for things that are currently in my existing mod
[12:14 AM] Wilko: i need to be able to load/compile new scripts the same way the game does it (atm I'm automatically adding all my shit to the end of an existing file)
[12:15 AM] Wilko: overwrite an existing game script as it's loaded in, so I can completely change the way a weapon works
[12:16 AM] Wilko: and maybe call an initialization function from c++, but I don't know how difficult it would be finding where they're called from
[12:17 AM] Wilko: and it's less necessary if I can overwrite files since I can overwrite a file to put the init in the script, just adds some limitations
*/

// TODO: Add a hook for the script error function (not just compile error)

HookedFunc<void, double, float> _Host_RunFrame("engine.dll", "\x48\x8B\xC4\x48\x89\x58\x00\xF3\x0F\x11\x48\x00\xF2\x0F\x11\x40\x00", "xxxxxx?xxxx?xxxx?");

HookedFunc<SQInteger, HSQUIRRELVM> client_base_print("client.dll", "\x40\x53\x48\x83\xEC\x30\xBA\x00\x00\x00\x00\x48\x8B\xD9\xE8\x00\x00\x00\x00\x8B\x53\x68", "xxxxxxx????xxxx????xxx");
HookedFunc<SQInteger, HSQUIRRELVM> server_base_print("server.dll", "\x40\x53\x48\x83\xEC\x30\xBA\x00\x00\x00\x00\x48\x8B\xD9\xE8\x00\x00\x00\x00\x8B\x53\x68", "xxxxxxx????xxxx????xxx");

SigScanFunc<SQRESULT, HSQUIRRELVM, CompileBufferState*, const SQChar*, int, int> client_sq_compilebuffer("client.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x50\x41\x8B\xE9", "xxxx?xxxx?xxxx?xxxxxxxxxxxx");
SigScanFunc<SQRESULT, HSQUIRRELVM, CompileBufferState*, const SQChar*, int, int> server_sq_compilebuffer("server.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x50\x41\x8B\xE9", "xxxx?xxxx?xxxx?xxxxxxxxxxxx");

SigScanFunc<void, HSQUIRRELVM> client_sq_pushroottable("client.dll", "\x8B\x51\x68\x44\x8B\xC2\x49\xC1\xE0\x04\x8D\x42\x01\x4C\x03\x41\x70\x89\x41\x68\xF7\x81\xB8", "xxxxxxxxxxxxxxxxxxxxxxx");
SigScanFunc<void, HSQUIRRELVM> server_sq_pushroottable("server.dll", "\x8B\x51\x68\x44\x8B\xC2\x49\xC1\xE0\x04\x8D\x42\x01\x4C\x03\x41\x70\x89\x41\x68\xF7\x81\xB8", "xxxxxxxxxxxxxxxxxxxxxxx");

SigScanFunc<SQRESULT, HSQUIRRELVM, SQInteger, SQBool, SQBool> client_sq_call("client.dll", "\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x49\x89\x73\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x44\x8B\xF2", "xxxxxx?xxx?xxx?xxxxxxxx?xxx");
SigScanFunc<SQRESULT, HSQUIRRELVM, SQInteger, SQBool, SQBool> server_sq_call("server.dll", "\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x49\x89\x73\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x44\x8B\xF2", "xxxxxx?xxx?xxx?xxxxxxxx?xxx");

HookedFunc<SQRESULT, HSQUIRRELVM, const SQChar*, int, const SQChar*> client_execute_script("client.dll", "\x40\x53\x48\x83\xEC\x00\x49\x63\xC0\x48\x89\x54\x24\x00\x48\x89\x54\x24\x00\x48\x03\xC2\x48\x8B\xD1\x4C\x8D\x44\x24\x00\x48\x89\x44\x24\x00\x48\x8B\x41\x00\x48\x8B\x98\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\x45\x33\xC9", "xxxxx?xxxxxxx?xxxx?xxxxxxxxxx?xxxx?xxx?xxx????xxxx????xxx");
HookedFunc<SQRESULT, HSQUIRRELVM, const SQChar*, int, const SQChar*> server_execute_script("server.dll", "\x40\x53\x48\x83\xEC\x00\x49\x63\xC0\x48\x89\x54\x24\x00\x48\x89\x54\x24\x00\x48\x03\xC2\x48\x8B\xD1\x4C\x8D\x44\x24\x00\x48\x89\x44\x24\x00\x48\x8B\x41\x00\x48\x8B\x98\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\x45\x33\xC9", "xxxxx?xxxxxxx?xxxx?xxxxxxxxxx?xxxx?xxx?xxx????xxxx????xxx");

HookedFunc<void, HSQUIRRELVM, const SQChar*, const SQChar*, SQInteger, SQInteger> client_sqstd_compiler_error("client.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x41\x50", "xxxx?xxxx?xxxx?xxxx?xxxxx????xxxx");
HookedFunc<void, HSQUIRRELVM, const SQChar*, const SQChar*, SQInteger, SQInteger> server_sqstd_compiler_error("server.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x41\x50", "xxxx?xxxx?xxxx?xxxx?xxxxx????xxxx");

SigScanFunc<void> clientVMFinder("client.dll", "\x44\x8B\xC2\x48\x8B\xD1\x48\x8B\x0D\x00\x00\x00\x00", "xxxxxxxxx????");
SigScanFunc<void> serverVMFinder("server.dll", "\x48\x89\x5C\x24\x00\x55\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\xEA", "xxxx?xxxx?xxx????xxx");

HookedFunc<int64_t, void*, HSQUIRRELVM, CompileBufferState*, const SQChar*> server_prepare_script("server.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x00\x48\x8B\xF1\x4C\x89\x41\x00", "xxxx?xxxx?xxxx?xxxx?xxxxx?xxxxxx?");
HookedFunc<int64_t, void*, HSQUIRRELVM, CompileBufferState*, const SQChar*> client_prepare_script("client.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x00\x48\x8B\xF1\x4C\x89\x41\x00", "xxxx?xxxx?xxxx?xxxx?xxxxx?xxxxxx?");

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

#define WRAPPED_MEMBER(name) SDKMemberWrapper<decltype(&TTF2SDK::##name), &TTF2SDK::##name>::Call

SQRESULT ClientExecuteScriptHook(HSQUIRRELVM v, const SQChar* buf, int length, const SQChar* chunkName)
{
    auto logger = spdlog::get("logger");
    logger->debug("Client loading script: {}", chunkName);
    SQRESULT res;
    std::ifstream f(std::string("D:\\dev\\ttf2\\replacements\\") + chunkName);
    if (f.good())
    {
        logger->info("Loading replacement for {} in client context", chunkName);
        // TODO: Change this to just attempt to compile the script
        std::stringstream buffer;
        buffer << f.rdbuf();
        std::string newData = buffer.str();
        res = client_execute_script(v, newData.c_str(), newData.length(), chunkName);
    }
    else
    {
        //logger->debug(buf);
        res = client_execute_script(v, buf, length, chunkName);
    }

    logger->debug("client_execute_script result = {}", res);
    return res;
}

SQRESULT ServerExecuteScriptHook(HSQUIRRELVM v, const SQChar* buf, int length, const SQChar* chunkName)
{
    auto logger = spdlog::get("logger");
    logger->debug("Server loading script: {}", chunkName);
    SQRESULT res;
    std::ifstream f(std::string("D:\\dev\\ttf2\\replacements\\") + chunkName);
    if (f.good())
    {
        logger->info("Loading replacement for {} in server context", chunkName);
        std::stringstream buffer;
        buffer << f.rdbuf();
        std::string newData = buffer.str();
        res = server_execute_script(v, newData.c_str(), newData.length(), chunkName);
    }
    else
    {
        //logger->debug(buf);
        res = server_execute_script(v, buf, length, chunkName);
    }

    logger->debug("server_execute_script result = {}", res);
    return res;
}

void ClientCompilerErrorHook(HSQUIRRELVM v, const SQChar* sErr, const SQChar* sSource, SQInteger line, SQInteger column)
{
    auto logger = spdlog::get("logger");
    logger->error("CLIENT SCRIPT COMPILE ERROR");
    logger->error("{} line = ({}) column = ({}) error = {}", sSource, line, column, sErr);
}

void ServerCompilerErrorHook(HSQUIRRELVM v, const SQChar* sErr, const SQChar* sSource, SQInteger line, SQInteger column)
{
    auto logger = spdlog::get("logger");
    logger->error("SERVER SCRIPT COMPILE ERROR");
    logger->error("{} line = ({}) column = ({}) error = {}", sSource, line, column, sErr);
}

void findAndReplaceAll(std::string& data, const std::string& toSearch, const std::string& replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);

    // Repeat till end is reached
    while (pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + toSearch.size());
    }
}

std::string clientData;
CompileBufferState clientState(clientData);

int64_t ClientPrepareScriptHook(void* unk, HSQUIRRELVM v, CompileBufferState* compileState, const SQChar* chunkName)
{
    auto logger = spdlog::get("logger");
    logger->debug("Client prepare script for {}", chunkName);
    //logger->debug(compileState->buffer);
    std::ifstream f(std::string("D:\\dev\\ttf2\\replacements\\") + chunkName);
    if (f.good())
    {
        logger->info("Loading replacement for {} in client context", chunkName);
        std::stringstream buffer;
        buffer << f.rdbuf();
        clientData = buffer.str();
        findAndReplaceAll(clientData, "\r\n", "\n");
        clientState = CompileBufferState(clientData);
        return client_prepare_script(unk, v, &clientState, chunkName);
    }
    else
    {
        //logger->debug(buf);
        return client_prepare_script(unk, v, compileState, chunkName);
    }
}

std::string serverData;
CompileBufferState serverState(serverData);

int64_t ServerPrepareScriptHook(void* unk, HSQUIRRELVM v, CompileBufferState* compileState, const SQChar* chunkName)
{
    auto logger = spdlog::get("logger");
    logger->debug("Server prepare script for {}", chunkName);
    //logger->debug(compileState->buffer);
    std::ifstream f(std::string("D:\\dev\\ttf2\\replacements\\") + chunkName);
    if (f.good())
    {
        logger->info("Loading replacement for {} in server context", chunkName);
        std::stringstream buffer;
        buffer << f.rdbuf();
        serverData = buffer.str();
        findAndReplaceAll(serverData, "\r\n", "\n");
        serverState = CompileBufferState(serverData);
        return server_prepare_script(unk, v, &serverState, chunkName);
    }
    else
    {
        //logger->debug(buf);
        return server_prepare_script(unk, v, compileState, chunkName);
    }
}

TTF2SDK::TTF2SDK()
{
    m_logger = spdlog::get("logger");

    if (MH_Initialize() != MH_OK)
    {
        throw std::exception("Failed to initialise MinHook");
    }

    SigScanFuncRegistry::GetInstance().ResolveAll();

    // Resolve pointer to VM structure in client.dll
    char* funcBase = (char*)clientVMFinder.GetFuncPtr();
    int offset = *(int*)(funcBase + 9);
    m_ppClientVM = (ClientVM**)(funcBase + 13 + offset);

    // Resolve pointer to VM structure in server.dll
    funcBase = (char*)serverVMFinder.GetFuncPtr();
    offset = *(int*)(funcBase + 13);
    m_ppServerVM = (ServerVM**)(funcBase + 17 + offset);

    // Hook functions
    _Host_RunFrame.Hook(WRAPPED_MEMBER(RunFrameHook));
    client_base_print.Hook(WRAPPED_MEMBER(ClientBasePrintHook));
    server_base_print.Hook(WRAPPED_MEMBER(ServerBasePrintHook));
    //client_execute_script.Hook(ClientExecuteScriptHook);
    //server_execute_script.Hook(ServerExecuteScriptHook);

    client_sqstd_compiler_error.Hook(ClientCompilerErrorHook);
    server_sqstd_compiler_error.Hook(ServerCompilerErrorHook);

    client_prepare_script.Hook(ClientPrepareScriptHook);
    server_prepare_script.Hook(ServerPrepareScriptHook);
}

void TTF2SDK::RunFrameHook(double absTime, float frameTime)
{
    if (m_shouldRunServerCode)
    {
        auto v = GetServerSQVM();
        if (v != nullptr)
        {
            std::string code = GetServerCode();
            m_logger->info("Executing SERVER code: {}", code);
            CompileBufferState s(code);
            SQRESULT compileRes = server_sq_compilebuffer(v, &s, "console", -1, 1);
            m_logger->debug("sq_compilebuffer returned {}", compileRes);
            if (SQ_SUCCEEDED(compileRes)) {
                server_sq_pushroottable(v);
                SQRESULT callRes = server_sq_call(v, 1, SQFalse, SQFalse);
                m_logger->debug("sq_call returned {}", callRes);
            }
            
            //SQRESULT res = server_execute_script(v, code.c_str(), code.size(), "console_script");
            //m_logger->debug("res = {}", res);
        }
        else
        {
            m_logger->error("Cannot execute SERVER code, no handle to squirrel VM");
            m_shouldRunServerCode = false;
        }
    }

    if (m_shouldRunClientCode)
    {
        auto v = GetClientSQVM();
        if (v != nullptr)
        {
            std::string code = GetClientCode();
            m_logger->info("Executing CLIENT code: {}", code);
            CompileBufferState s(code);
            SQRESULT compileRes = client_sq_compilebuffer(v, &s, "console", -1, 1);
            m_logger->debug("sq_compilebuffer returned {}", compileRes);
            if (SQ_SUCCEEDED(compileRes)) {
                client_sq_pushroottable(v);
                SQRESULT callRes = client_sq_call(v, 1, SQFalse, SQFalse);
                m_logger->debug("sq_call returned {}", callRes);
            }
        }
        else
        {
            m_logger->error("Cannot execute CLIENT code, no handle to squirrel VM");
            m_shouldRunClientCode = false;
        }
    }

    return _Host_RunFrame(absTime, frameTime);
}

SQInteger TTF2SDK::ClientBasePrintHook(HSQUIRRELVM v)
{
    static auto printFuncLambda = [](HSQUIRRELVM v, const SQChar* s, ...) {
        va_list vl;
        va_start(vl, s);
        g_SDK->PrintFunc(v, "CLIENT", s, vl);
        va_end(vl);
    };

    SQPRINTFUNCTION oldPrintFunc = _ss(v)->_printfunc;
    _ss(v)->_printfunc = printFuncLambda;
    SQInteger res = client_base_print(v);
    _ss(v)->_printfunc = oldPrintFunc;

    return res;
}

SQInteger TTF2SDK::ServerBasePrintHook(HSQUIRRELVM v)
{
    static auto printFuncLambda = [](HSQUIRRELVM v, const SQChar* s, ...) {
        va_list vl;
        va_start(vl, s);
        g_SDK->PrintFunc(v, "SERVER", s, vl);
        va_end(vl);
    };

    SQPRINTFUNCTION oldPrintFunc = _ss(v)->_printfunc;
    _ss(v)->_printfunc = printFuncLambda;
    SQInteger res = server_base_print(v);
    _ss(v)->_printfunc = oldPrintFunc;

    return res;
}

void TTF2SDK::PrintFunc(HSQUIRRELVM v, const SQChar* source, const SQChar* s, va_list args)
{
    SQChar buf[1024];

    int charactersWritten = _vsnprintf_s(buf, _TRUNCATE, s, args);
    if (charactersWritten > 0)
    {
        if (buf[charactersWritten - 1] == '\n')
        {
            buf[charactersWritten - 1] = 0;
        }

        m_logger->info("{}: {}", source, buf);
    }
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

struct ThreadSuspender
{
    ThreadSuspender()
    {
        Util::SuspendAllOtherThreads();
    }

    ~ThreadSuspender()
    {
        Util::ResumeAllOtherThreads();
    }
};

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
        ThreadSuspender suspender;
        g_SDK = std::make_unique<TTF2SDK>();
        return true;
    }
    catch (std::exception& ex)
    {
        spdlog::get("logger")->critical("Failed to initialise SDK: {}", ex.what());
        return false;
    }
}
