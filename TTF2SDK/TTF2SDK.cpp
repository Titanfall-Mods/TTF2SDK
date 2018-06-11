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

HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateGeometryShader), &ID3D11DeviceVtbl::CreateGeometryShader> ID3D11Device_CreateGeometryShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreatePixelShader), &ID3D11DeviceVtbl::CreatePixelShader> ID3D11Device_CreatePixelShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateVertexShader), &ID3D11DeviceVtbl::CreateVertexShader> ID3D11Device_CreateVertexShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateComputeShader), &ID3D11DeviceVtbl::CreateComputeShader> ID3D11Device_CreateComputeShader;

HookedFunc<bool, char*> LoadPakForLevel("engine.dll", "\x48\x81\xEC\x00\x00\x00\x00\x48\x8D\x54\x24\x00\x41\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x4C\x24\x00", "xxx????xxxx?xx????x????xxxx?");

SigScanFunc<void> mpJumpPatchFinder("engine.dll", "\x75\x00\x44\x8D\x40\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00", "x?xxx?xxx????xxx????x????");
HookedFunc<int64_t, const char*, const char*, int64_t> engineCompareFunc("engine.dll", "\x4D\x8B\xD0\x4D\x85\xC0", "xxxxxx");
SigScanFunc<void> secondMpJumpPatchFinder("engine.dll", "\x0F\x84\x00\x00\x00\x00\x84\xDB\x74\x00\x48\x8B\x0D\x00\x00\x00\x00", "xx????xxx?xxx????");

std::atomic_bool inLoad = false;
std::atomic_bool overrideShaders = false;
ID3D11GeometryShader* geometryShader = NULL;
ID3D11PixelShader* pixelShader = NULL;
ID3D11ComputeShader* computeShader = NULL;
ID3D11VertexShader* vertexShader = NULL;

HRESULT STDMETHODCALLTYPE CreateVertexShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader)
{
    if (inLoad && overrideShaders)
    {
        *ppVertexShader = vertexShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreateVertexShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
    }
}

HRESULT STDMETHODCALLTYPE CreateGeometryShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
{
    if (inLoad && overrideShaders)
    {
        *ppGeometryShader = geometryShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreateGeometryShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppGeometryShader);
    }
}

HRESULT STDMETHODCALLTYPE CreatePixelShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader)
{
    if (inLoad && overrideShaders)
    {
        *ppPixelShader = pixelShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreatePixelShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);
    }
}

HRESULT STDMETHODCALLTYPE CreateComputeShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader)
{
    if (inLoad && overrideShaders)
    {
        *ppComputeShader = computeShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreateComputeShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader);
    }
}

void TTF2SDK::compileShaders()
{
    const char* shaderText = "void main() { return; }";
    const char* geometryShaderText = "struct GS_INPUT {}; [maxvertexcount(4)] void main(point GS_INPUT a[1]) { return; }";
    const char* computeShaderText = "[numthreads(1, 1, 1)] void main() { return; }";

    ID3DBlob* vertexShaderBlob = NULL;
    HRESULT result = D3DCompile(
        shaderText,
        strlen(shaderText),
        "TTF2SDK_VS",
        NULL,
        NULL,
        "main",
        "vs_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &vertexShaderBlob,
        NULL
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to compile vertex shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Vertex shader blob: {}", (void*)vertexShaderBlob);
    }

    ID3DBlob* pixelShaderBlob = NULL;
    result = D3DCompile(
        shaderText,
        strlen(shaderText),
        "TTF2SDK_PS",
        NULL,
        NULL,
        "main",
        "ps_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &pixelShaderBlob,
        NULL
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to compile pixel shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Pixel shader blob: {}", (void*)pixelShaderBlob);
    }

    ID3DBlob* geometryShaderBlob = NULL;
    result = D3DCompile(
        geometryShaderText,
        strlen(geometryShaderText),
        "TTF2SDK_GS",
        NULL,
        NULL,
        "main",
        "gs_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &geometryShaderBlob,
        NULL
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to compile geometry shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Geometry shader blob: {}", (void*)geometryShaderBlob);
    }

    ID3DBlob* computeShaderBlob = NULL;
    result = D3DCompile(
        computeShaderText,
        strlen(computeShaderText),
        "TTF2SDK_CS",
        NULL,
        NULL,
        "main",
        "cs_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &computeShaderBlob,
        NULL
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to compile compute shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Compute shader blob: {}", (void*)computeShaderBlob);
    }

    /*
    ID3D11Device* dev = *m_ppD3D11Device;

    geometryShader = NULL;
    result = dev->lpVtbl->CreateGeometryShader(
        dev,
        geometryShaderBlob->lpVtbl->GetBufferPointer(geometryShaderBlob),
        geometryShaderBlob->lpVtbl->GetBufferSize(geometryShaderBlob),
        NULL,
        &geometryShader
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to create geometry shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Geometry shader: {}", (void*)geometryShader);
    }

    vertexShader = NULL;
    result = dev->lpVtbl->CreateVertexShader(
        dev,
        vertexShaderBlob->lpVtbl->GetBufferPointer(vertexShaderBlob),
        vertexShaderBlob->lpVtbl->GetBufferSize(vertexShaderBlob),
        NULL,
        &vertexShader
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to create vertex shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Vertex shader: {}", (void*)vertexShader);
    }

    pixelShader = NULL;
    result = dev->lpVtbl->CreatePixelShader(
        dev,
        pixelShaderBlob->lpVtbl->GetBufferPointer(pixelShaderBlob),
        pixelShaderBlob->lpVtbl->GetBufferSize(pixelShaderBlob),
        NULL,
        &pixelShader
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to create pixel shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Pixel shader: {}", (void*)pixelShader);
    }

    computeShader = NULL;
    result = dev->lpVtbl->CreateComputeShader(
        dev,
        computeShaderBlob->lpVtbl->GetBufferPointer(computeShaderBlob),
        computeShaderBlob->lpVtbl->GetBufferSize(computeShaderBlob),
        NULL,
        &computeShader
    );

    if (!SUCCEEDED(result))
    {
        spdlog::get("logger")->error("Failed to create compute shader");
        return;
    }
    else
    {
        spdlog::get("logger")->info("Compute shader: {}", (void*)computeShader);
    }
    */
}

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
    m_engineServer("engine.dll", "VEngineServer022"),
    m_engineClient("engine.dll", "VEngineClient013")
{
    m_logger = spdlog::get("logger");

    SigScanFuncRegistry::GetInstance().ResolveAll();

    if (MH_Initialize() != MH_OK)
    {
        throw std::exception("Failed to initialise MinHook");
    }

    m_conCommandManager.reset(new ConCommandManager());

    m_fsManager.reset(new FileSystemManager(settings.BasePath, *m_conCommandManager));
    m_sqManager.reset(new SquirrelManager(*m_conCommandManager));
    m_pakManager.reset(new PakManager(*m_conCommandManager, m_engineServer, *m_sqManager));
    m_modManager.reset(new ModManager(*m_conCommandManager));
    m_uiManager.reset(new UIManager(*m_conCommandManager, *m_sqManager));

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

SourceInterface<IVEngineServer>& TTF2SDK::GetEngineServer()
{
    return m_engineServer;
}

SourceInterface<IVEngineClient>& TTF2SDK::GetEngineClient()
{
    return m_engineClient;
}

void TTF2SDK::RunFrameHook(double absTime, float frameTime)
{
    for (const auto& frameTask : m_frameTasks)
    {
        frameTask->RunFrame();
    }

    m_frameTasks.erase(std::remove_if(m_frameTasks.begin(), m_frameTasks.end(), [](const std::unique_ptr<IFrameTask>& t)
    { 
        return t->IsFinished();
    }), m_frameTasks.end());

    static bool called = false;
    if (!called)
    {
        m_logger->warn("RunFrame called for the first time");
        m_pakManager->PreloadAllPaks();
        called = true;
    }
   
    return _Host_RunFrame(absTime, frameTime);
}

void TTF2SDK::AddFrameTask(std::unique_ptr<IFrameTask> task)
{
    m_frameTasks.push_back(std::move(task));
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
