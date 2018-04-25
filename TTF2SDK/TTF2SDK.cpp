#include "stdafx.h"

std::unique_ptr<Console> g_console;
std::unique_ptr<TTF2SDK> g_SDK;

/*
[12:13 AM] Wilko: so for things that are currently in my existing mod
[12:14 AM] Wilko: i need to be able to load/compile new scripts the same way the game does it (atm I'm automatically adding all my shit to the end of an existing file)
[12:15 AM] Wilko: overwrite an existing game script as it's loaded in, so I can completely change the way a weapon works
[12:16 AM] Wilko: and maybe call an initialization function from c++, but I don't know how difficult it would be finding where they're called from
[12:17 AM] Wilko: and it's less necessary if I can overwrite files since I can overwrite a file to put the init in the script, just adds some limitations
the two requests btw are "ability to load a VPK manually"
[12:18 AM] Wilko: so I can load props from other levels, and "override weapon file (not the squirrel one)"
*/

// TODO: Add a hook for the script error function (not just compile error)
// TODO: Hook CoreMsgV
// g_pakLoadApi[9] <--- gets called with the pointer to the pak struct, the qword thing, and a function. i'm guessing the function is a callback for when it's done or progress or something?
// g_pakLoadApi[6] <--- i reckon it's probably a thing to release it or something
// g_pakLoadApi[3] <--- return pointer to initialised pak data structure (const char* name, void* allocatorFunctionTable, int someInt)
// g_pakLoadApi[4] <-- do 3 and 9 in the same step.
// result = (char *)g_pakLoadApi[4](
//    Dest,
//    off_7FFD14F55E20,
//    7i64,
//    (__int64)qword_7FFD271A1F00,
//    (__int64(__fastcall *)())nullsub_41);

// so i need: qword_7FFD271A1F00 and off_7FFD14F55E20

HookedFunc<void, double, float> _Host_RunFrame("engine.dll", "\x48\x8B\xC4\x48\x89\x58\x00\xF3\x0F\x11\x48\x00\xF2\x0F\x11\x40\x00", "xxxxxx?xxxx?xxxx?");

SharedHookedFunc<SQInteger, HSQUIRRELVM> base_print("\x40\x53\x48\x83\xEC\x30\xBA\x00\x00\x00\x00\x48\x8B\xD9\xE8\x00\x00\x00\x00\x8B\x53\x68", "xxxxxxx????xxxx????xxx");
SharedSigFunc<SQRESULT, HSQUIRRELVM, CompileBufferState*, const SQChar*, int, int> sq_compilebuffer("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x50\x41\x8B\xE9", "xxxx?xxxx?xxxx?xxxxxxxxxxxx");
SharedSigFunc<void, HSQUIRRELVM> sq_pushroottable("\x8B\x51\x68\x44\x8B\xC2\x49\xC1\xE0\x04\x8D\x42\x01\x4C\x03\x41\x70\x89\x41\x68\xF7\x81\xB8", "xxxxxxxxxxxxxxxxxxxxxxx");
SharedSigFunc<SQRESULT, HSQUIRRELVM, SQInteger, SQBool, SQBool> sq_call("\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x49\x89\x73\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x44\x8B\xF2", "xxxxxx?xxx?xxx?xxxxxxxx?xxx");
SharedHookedFunc<void, HSQUIRRELVM, const SQChar*, const SQChar*, SQInteger, SQInteger> sqstd_compiler_error("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x41\x50", "xxxx?xxxx?xxxx?xxxx?xxxxx????xxxx");

HookedVTableFunc<decltype(&IFileSystem::VTable::AddSearchPath), &IFileSystem::VTable::AddSearchPath> IFileSystem_AddSearchPath;
HookedVTableFunc<decltype(&IFileSystem::VTable::ReadFromCache), &IFileSystem::VTable::ReadFromCache> IFileSystem_ReadFromCache;
HookedVTableFunc<decltype(&IFileSystem::VTable::MountVPK), &IFileSystem::VTable::MountVPK> IFileSystem_MountVPK;

HookedVTableFunc<decltype(&IEngineServer::VTable::SpewFunc), &IEngineServer::VTable::SpewFunc> IEngineServer_SpewFunc;

HookedFunc<__int32*, VPKInfo*, __int32*, char*> ReadFileFromVPK("filesystem_stdio.dll", "\x48\x89\x5C\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x49\x8B\xC0\x48\x8B\xDA", "xxxx?xxxx????xxxxxx");

SigScanFunc<void> clientVMFinder("client.dll", "\x44\x8B\xC2\x48\x8B\xD1\x48\x8B\x0D\x00\x00\x00\x00", "xxxxxxxxx????");
SigScanFunc<void> serverVMFinder("server.dll", "\x48\x89\x5C\x24\x00\x55\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\xEA", "xxxx?xxxx?xxx????xxx");

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

__int64 SpewFuncHook(IEngineServer* engineServer, SpewType_t type, const char* format, va_list args)
{
    char pTempBuffer[5020];

    int val = _vsnprintf_s(pTempBuffer, sizeof(pTempBuffer) - 1, format, args); // TODO: maybe use something else
    if (val == -1)
    {
        spdlog::get("logger")->warn("Failed to call _vsnprintf for SpewFunc");
        return IEngineServer_SpewFunc(engineServer, type, format, args);
    }

    if (type == SPEW_MESSAGE)
    {
        spdlog::get("logger")->info("SERVER (SPEW_MESSAGE): {}", pTempBuffer);
    }
    else if (type == SPEW_WARNING)
    {
        spdlog::get("logger")->warn("SERVER (SPEW_WARNING): {}", pTempBuffer);
    }
    else
    {
        spdlog::get("logger")->info("SERVER ({}): {}", type, pTempBuffer);
    }

    return IEngineServer_SpewFunc(engineServer, type, format, args);
}

TTF2SDK::TTF2SDK()
    : m_replacementManager("D:\\dev\\ttf2\\searchpath\\"), // TODO: make this a configuration setting. Also make sure it has a trailing slash.
      m_fileSystem("filesystem_stdio.dll", "VFileSystem017"),
      m_engineClient("engine.dll", "VEngineClient013"),
      m_engineServer("engine.dll", "VEngineServer022")
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
    IFileSystem_AddSearchPath.Hook(m_fileSystem->m_vtable, WRAPPED_MEMBER(AddSearchPathHook));
    IFileSystem_ReadFromCache.Hook(m_fileSystem->m_vtable, WRAPPED_MEMBER(ReadFromCacheHook));
    IFileSystem_MountVPK.Hook(m_fileSystem->m_vtable, WRAPPED_MEMBER(MountVPKHook));
    ReadFileFromVPK.Hook(WRAPPED_MEMBER(ReadFileFromVPKHook));

    IEngineServer_SpewFunc.Hook(m_engineServer->m_vtable, SpewFuncHook);

    base_print.Hook(WRAPPED_MEMBER(BasePrintHook<CONTEXT_CLIENT>), WRAPPED_MEMBER(BasePrintHook<CONTEXT_SERVER>));
    sqstd_compiler_error.Hook(WRAPPED_MEMBER(CompilerErrorHook<CONTEXT_CLIENT>), WRAPPED_MEMBER(CompilerErrorHook<CONTEXT_SERVER>));

    _Host_RunFrame.Hook(WRAPPED_MEMBER(RunFrameHook));
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
            SQRESULT compileRes = sq_compilebuffer.CallServer(v, &s, "console", -1, 1);
            m_logger->debug("sq_compilebuffer returned {}", compileRes);
            if (SQ_SUCCEEDED(compileRes)) {
                sq_pushroottable.CallServer(v);
                SQRESULT callRes = sq_call.CallServer(v, 1, SQFalse, SQFalse);
                m_logger->debug("sq_call returned {}", callRes);
            }
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
            SQRESULT compileRes = sq_compilebuffer.CallClient(v, &s, "console", -1, 1);
            m_logger->debug("sq_compilebuffer returned {}", compileRes);
            if (SQ_SUCCEEDED(compileRes)) {
                sq_pushroottable.CallClient(v);
                SQRESULT callRes = sq_call.CallClient(v, 1, SQFalse, SQFalse);
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

void TTF2SDK::AddSearchPathHook(IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType)
{
    m_logger->debug("IFileSystem::AddSearchPath: path = {}, pathID = {}, addType = {}", pPath, pathID != nullptr ? pathID : "", addType);

    // Add the path as intended
    IFileSystem_AddSearchPath(fileSystem, pPath, pathID, addType);

    // Add our search path to the head again to make sure we're first
    IFileSystem_AddSearchPath(fileSystem, m_replacementManager.GetSearchPath().c_str(), "GAME", PATH_ADD_TO_HEAD);
}

bool TTF2SDK::ReadFromCacheHook(IFileSystem* fileSystem, const char* path, void* result)
{
    // If the path is one of our replacements, we will not allow the cache to respond
    if (m_replacementManager.ShouldReplaceFile(path))
    {
        m_logger->info("IFileSystem::ReadFromCache: blocking cache response for {}", path);
        return false;
    }

    bool res = IFileSystem_ReadFromCache(fileSystem, path, result);
    m_logger->debug("IFileSystem::ReadFromCache: path = {}, res = {}", path, res);
    return res;
}

__int32* TTF2SDK::ReadFileFromVPKHook(VPKInfo* vpkInfo, __int32* b, char* filename)
{
    static bool addedSearch = false;
    if (!addedSearch)
    {
        m_logger->info("ReadFileFromVPK: adding replacements search path ({})", m_replacementManager.GetSearchPath());
        IFileSystem_AddSearchPath(m_fileSystem, m_replacementManager.GetSearchPath().c_str(), "GAME", PATH_ADD_TO_HEAD);
        addedSearch = true;
    }

    // If the path is one of our replacements, we will not allow the cache to respond
    if (m_replacementManager.ShouldReplaceFile(filename))
    {
        m_logger->info("ReadFileFromVPK: blocking response for {} from {}", filename, vpkInfo->path);
        *b = -1;
        return b;
    }

    __int32* result = ReadFileFromVPK(vpkInfo, b, filename);
    m_logger->debug("ReadFileFromVPK: vpk = {}, file = {}, result = {}", vpkInfo->path, filename, *b);

    return result;
}

unsigned int* TTF2SDK::MountVPKHook(IFileSystem* fileSystem, const char* vpkPath)
{
    m_logger->debug("IFileSystem::MountVPK: vpkPath = {}", vpkPath);
    unsigned int* res = IFileSystem_MountVPK(fileSystem, vpkPath);
    return res;
}

template<ExecutionContext context>
SQInteger TTF2SDK::BasePrintHook(HSQUIRRELVM v)
{
    static auto printFuncLambda = [](HSQUIRRELVM v, const SQChar* s, ...) {
        va_list vl;
        va_start(vl, s);
        g_SDK->PrintFunc(v, Util::GetContextName(context), s, vl);
        va_end(vl);
    };

    SQPRINTFUNCTION oldPrintFunc = _ss(v)->_printfunc;
    _ss(v)->_printfunc = printFuncLambda;
    SQInteger res = base_print.Call<context>(v);
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

template<ExecutionContext context>
void TTF2SDK::CompilerErrorHook(HSQUIRRELVM v, const SQChar* sErr, const SQChar* sSource, SQInteger line, SQInteger column)
{
    m_logger->error("{} SCRIPT COMPILE ERROR", Util::GetContextName(context));
    m_logger->error("{} line = ({}) column = ({}) error = {}", sSource, line, column, sErr);
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
        Util::ThreadSuspender suspender;
        g_SDK = std::make_unique<TTF2SDK>();
        return true;
    }
    catch (std::exception& ex)
    {
        spdlog::get("logger")->critical("Failed to initialise SDK: {}", ex.what());
        return false;
    }
}
