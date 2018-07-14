#include "stdafx.h"

std::unique_ptr<Console> g_console;
std::unique_ptr<TTF2SDK> g_SDK;

TTF2SDK& SDK()
{
    return *g_SDK;
}

// TODO: Add a hook for the script error function (not just compile error)
// TODO: Hook CoreMsgV
// g_pakLoadApi[9] <--- gets called with the pointer to the pak struct, the qword thing, and a function. i'm guessing the function is a callback for when it's done or progress or something?
// g_pakLoadApi[6] <--- i reckon it's probably a thing to release it or something
// g_pakLoadApi[3] <--- return pointer to initialised pak data structure (const char* name, void* allocatorFunctionTable, int someInt)
// g_pakLoadApi[4] <-- do 3 and 9 in the same step.

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&TTF2SDK::##name), &TTF2SDK::##name, decltype(&SDK), &SDK>::Call

HookedFunc<void, double, float> _Host_RunFrame("engine.dll", "\x48\x8B\xC4\x48\x89\x58\x00\xF3\x0F\x11\x48\x00\xF2\x0F\x11\x40\x00", "xxxxxx?xxxx?xxxx?");
HookedVTableFunc<decltype(&IVEngineServer::VTable::SpewFunc), &IVEngineServer::VTable::SpewFunc> IVEngineServer_SpewFunc;
SigScanFunc<void> d3d11DeviceFinder("materialsystem_dx11.dll", "\x48\x83\xEC\x00\x33\xC0\x89\x54\x24\x00\x4C\x8B\xC9\x48\x8B\x0D\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00", "xxx?xxxxx?xxxxxx????xxx?????");
SigScanFunc<void> mpJumpPatchFinder("engine.dll", "\x75\x00\x44\x8D\x40\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00", "x?xxx?xxx????xxx????x????");
HookedFunc<int64_t, const char*, const char*, int64_t> engineCompareFunc("engine.dll", "\x4D\x8B\xD0\x4D\x85\xC0", "xxxxxx");
SigScanFunc<void> secondMpJumpPatchFinder("engine.dll", "\x0F\x84\x00\x00\x00\x00\x84\xDB\x74\x00\x48\x8B\x0D\x00\x00\x00\x00", "xx????xxx?xxx????");

__int64 SpewFuncHook(IVEngineServer* engineServer, SpewType_t type, const char* format, va_list args)
{
    char pTempBuffer[5020];

    int val = _vsnprintf_s(pTempBuffer, sizeof(pTempBuffer) - 1, format, args); // TODO: maybe use something else
    if (val == -1)
    {
        spdlog::get("logger")->warn("Failed to call _vsnprintf for SpewFunc");
        return IVEngineServer_SpewFunc(engineServer, type, format, args);
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

    return IVEngineServer_SpewFunc(engineServer, type, format, args);
}

int64_t compareFuncHook(const char* first, const char* second, int64_t count)
{
    if (strcmp(second, "mp_") == 0 && strncmp(first, "mp_", 3) == 0)
    {
        SPDLOG_TRACE(spdlog::get("logger"), "Overwriting result of compareFunc for {}", first);
        return 1;
    }
    else
    {
        return engineCompareFunc(first, second, count);
    }
}

TTF2SDK::TTF2SDK(const SDKSettings& settings) :
	Settings( &settings ),
    m_engineServer("engine.dll", "VEngineServer022"),
    m_engineClient("engine.dll", "VEngineClient013"),
    m_inputSystem("inputsystem.dll", "InputSystemVersion001")
{
    m_logger = spdlog::get("logger");

    SigScanFuncRegistry::GetInstance().ResolveAll();

    if (MH_Initialize() != MH_OK)
    {
        throw std::exception("Failed to initialise MinHook");
    }

    // Get pointer to d3d device
    char* funcBase = (char*)d3d11DeviceFinder.GetFuncPtr();
    int offset = *(int*)(funcBase + 16);
    m_ppD3D11Device = (ID3D11Device**)(funcBase + 20 + offset);

    SPDLOG_DEBUG(m_logger, "m_ppD3D11Device = {}", (void*)m_ppD3D11Device);
    SPDLOG_DEBUG(m_logger, "pD3D11Device = {}", (void*)*m_ppD3D11Device);

    m_conCommandManager.reset(new ConCommandManager());

    m_fsManager.reset(new FileSystemManager(settings.BasePath, *m_conCommandManager));
    m_sqManager.reset(new SquirrelManager(*m_conCommandManager));
    m_uiManager.reset(new UIManager(*m_conCommandManager, *m_sqManager, *m_fsManager, m_ppD3D11Device));
    m_pakManager.reset(new PakManager(*m_conCommandManager, m_engineServer, *m_sqManager, m_ppD3D11Device));
    m_modManager.reset(new ModManager(*m_conCommandManager));

    m_icepickMenu.reset(new IcepickMenu(*m_conCommandManager, *m_uiManager, *m_sqManager));

    IVEngineServer_SpewFunc.Hook(m_engineServer->m_vtable, SpewFuncHook);
    _Host_RunFrame.Hook(WRAPPED_MEMBER(RunFrameHook));
    engineCompareFunc.Hook(compareFuncHook);

    // Patch jump for loading MP maps in single player
    {
        void* ptr = mpJumpPatchFinder.GetFuncPtr();
        SPDLOG_DEBUG(m_logger, "mpJumpPatchFinder = {}", ptr);
        TempReadWrite rw(ptr);
        *(unsigned char*)ptr = 0xEB;
    }

    // Second patch, changing jz to jnz
    {
        void* ptr = secondMpJumpPatchFinder.GetFuncPtr();
        SPDLOG_DEBUG(m_logger, "secondMpJumpPatchFinder = {}", ptr);
        TempReadWrite rw(ptr);
        *((unsigned char*)ptr + 1) = 0x85;
    }

    // Add delayed func task
    m_delayedFuncTask = std::make_shared<DelayedFuncTask>();
    AddFrameTask(m_delayedFuncTask);

    // Add squirrel functions for mouse deltas
    m_sqManager->AddFuncRegistration(CONTEXT_CLIENT, "int", "GetMouseDeltaX", "", "", WRAPPED_MEMBER(SQGetMouseDeltaX));
    m_sqManager->AddFuncRegistration(CONTEXT_CLIENT, "int", "GetMouseDeltaY", "", "", WRAPPED_MEMBER(SQGetMouseDeltaY));
}

FileSystemManager& TTF2SDK::GetFSManager()
{
    return *m_fsManager;
}

SquirrelManager& TTF2SDK::GetSQManager()
{
    return *m_sqManager;
}

PakManager& TTF2SDK::GetPakManager()
{
    return *m_pakManager;
}

ModManager& TTF2SDK::GetModManager()
{
    return *m_modManager;
}

ConCommandManager& TTF2SDK::GetConCommandManager()
{
    return *m_conCommandManager;
}

UIManager& TTF2SDK::GetUIManager()
{
    return *m_uiManager;
}

ID3D11Device** TTF2SDK::GetD3D11DevicePtr()
{
    return m_ppD3D11Device;
}

IcepickMenu& TTF2SDK::GetIcepickMenu()
{
    return *m_icepickMenu;
}

SourceInterface<IVEngineServer>& TTF2SDK::GetEngineServer()
{
    return m_engineServer;
}

SourceInterface<IVEngineClient>& TTF2SDK::GetEngineClient()
{
    return m_engineClient;
}

SourceInterface<IInputSystem>& TTF2SDK::GetInputSystem()
{
    return m_inputSystem;
}

void TTF2SDK::RunFrameHook(double absTime, float frameTime)
{
    for (const auto& frameTask : m_frameTasks)
    {
        frameTask->RunFrame();
    }

    m_frameTasks.erase(std::remove_if(m_frameTasks.begin(), m_frameTasks.end(), [](const std::shared_ptr<IFrameTask>& t)
    { 
        return t->IsFinished();
    }), m_frameTasks.end());

    static bool called = false;
    if (!called)
    {
        m_logger->info("RunFrame called for the first time");
        m_pakManager->PreloadAllPaks();
        called = true;
    }
   
    return _Host_RunFrame(absTime, frameTime);
}

void TTF2SDK::AddFrameTask(std::shared_ptr<IFrameTask> task)
{
    m_frameTasks.push_back(std::move(task));
}

void TTF2SDK::AddDelayedFunc(std::function<void()> func, int frames)
{
    m_delayedFuncTask->AddFunc(func, frames);
}

SQInteger TTF2SDK::SQGetMouseDeltaX(HSQUIRRELVM v)
{
    sq_pushinteger.CallClient(v, m_inputSystem->m_analogDeltaX);
    return 1;
}

SQInteger TTF2SDK::SQGetMouseDeltaY(HSQUIRRELVM v)
{
    sq_pushinteger.CallClient(v, m_inputSystem->m_analogDeltaY);
    return 1;
}

TTF2SDK::~TTF2SDK()
{
    // TODO: Reorder these
    m_sqManager.reset();
    m_conCommandManager.reset();
    m_fsManager.reset();
    m_pakManager.reset();
    m_modManager.reset();
    m_uiManager.reset();
    
    MH_Uninitialize();
}

void SetupLogger(const std::string& filename)
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
#ifdef _DEBUG
    logger->set_level(spdlog::level::trace);
#else
    logger->set_level(spdlog::level::debug);
#endif

    if (fileError)
    {
        logger->warn("Failed to initialise file sink, log file will be unavailable ({})", *fileError);
    }

    spdlog::register_logger(logger);
}

bool SetupSDK(const SDKSettings& settings)
{
    // Separate try catch because these are required for logging to work
    try
    {
        g_console = std::make_unique<Console>();
        fs::path basePath(settings.BasePath);
        SetupLogger((basePath / "TTF2SDK.log").string());
    }
    catch (std::exception)
    {
        return false;
    }

    try
    {
        // TODO: Make this smarter (automatically pull DLL we need to load from somewhere)
        Util::WaitForModuleHandle("engine.dll");
        Util::WaitForModuleHandle("client.dll");
        Util::WaitForModuleHandle("server.dll");
        Util::WaitForModuleHandle("vstdlib.dll");
        Util::WaitForModuleHandle("filesystem_stdio.dll");
        Util::WaitForModuleHandle("rtech_game.dll");
        Util::WaitForModuleHandle("studiorender.dll");
        Util::WaitForModuleHandle("materialsystem_dx11.dll");

        Util::ThreadSuspender suspender;
        g_SDK = std::make_unique<TTF2SDK>(settings);
        return true;
    }
    catch (std::exception& ex)
    {
        spdlog::get("logger")->critical("Failed to initialise SDK: {}", ex.what());
        return false;
    }
}

void FreeSDK()
{
    g_SDK.reset();
}
