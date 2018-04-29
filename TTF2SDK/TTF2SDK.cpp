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

struct AllocFuncs
{
    void* (*allocFunc)(__int64 a1, size_t a2, size_t a3);
    void(*freeFunc)(__int64 a1, void* a2);
};


HookedFunc<void, double, float> _Host_RunFrame("engine.dll", "\x48\x8B\xC4\x48\x89\x58\x00\xF3\x0F\x11\x48\x00\xF2\x0F\x11\x40\x00", "xxxxxx?xxxx?xxxx?");

SharedHookedFunc<SQInteger, HSQUIRRELVM> base_print("\x40\x53\x48\x83\xEC\x30\xBA\x00\x00\x00\x00\x48\x8B\xD9\xE8\x00\x00\x00\x00\x8B\x53\x68", "xxxxxxx????xxxx????xxx");
SharedSigFunc<SQRESULT, HSQUIRRELVM, CompileBufferState*, const SQChar*, int, int> sq_compilebuffer("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x50\x41\x8B\xE9", "xxxx?xxxx?xxxx?xxxxxxxxxxxx");
SharedSigFunc<void, HSQUIRRELVM> sq_pushroottable("\x8B\x51\x68\x44\x8B\xC2\x49\xC1\xE0\x04\x8D\x42\x01\x4C\x03\x41\x70\x89\x41\x68\xF7\x81\xB8", "xxxxxxxxxxxxxxxxxxxxxxx");
SharedSigFunc<SQRESULT, HSQUIRRELVM, SQInteger, SQBool, SQBool> sq_call("\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x49\x89\x73\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x44\x8B\xF2", "xxxxxx?xxx?xxx?xxxxxxxx?xxx");
SharedHookedFunc<void, HSQUIRRELVM, const SQChar*, const SQChar*, SQInteger, SQInteger> sqstd_compiler_error("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x41\x50", "xxxx?xxxx?xxxx?xxxx?xxxxx????xxxx");

HookedFunc<__int64, void*, void*> Studio_LoadModel("engine.dll", "\x48\x89\x6C\x24\x00\x41\x56\x48\x83\xEC\x00\x83\x8A\x00\x00\x00\x00\x00", "xxxx?xxxxx?xx?????");

HookedVTableFunc<decltype(&IFileSystem::VTable::AddSearchPath), &IFileSystem::VTable::AddSearchPath> IFileSystem_AddSearchPath;
HookedVTableFunc<decltype(&IFileSystem::VTable::ReadFromCache), &IFileSystem::VTable::ReadFromCache> IFileSystem_ReadFromCache;
HookedVTableFunc<decltype(&IFileSystem::VTable::MountVPK), &IFileSystem::VTable::MountVPK> IFileSystem_MountVPK;

HookedVTableFunc<decltype(&IEngineClient::VTable::FirstFuncOfInterest), &IEngineClient::VTable::FirstFuncOfInterest> IEngineClient_FirstFuncOfInterest;
HookedVTableFunc<decltype(&IEngineClient::VTable::SecondFuncOfInterest), &IEngineClient::VTable::SecondFuncOfInterest> IEngineClient_SecondFuncOfInterest;

HookedVTableFunc<decltype(&IEngineServer::VTable::SpewFunc), &IEngineServer::VTable::SpewFunc> IEngineServer_SpewFunc;

HookedFunc<unsigned __int64, __int64, signed int*, __int64, __int64, unsigned int> CStudioRenderContext_LoadMaterials("studiorender.dll", "\x40\x53\x55\x57", "xxxx");

HookedFunc<__int64, const char*, AllocFuncs*, int> pakFunc3("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF1\x48\x8D\x0D\x00\x00\x00\x00", "xxxx?xxxx?xxxx?xxxxxx????");
HookedFunc<__int64, __int64, void*, void*> pakFunc9("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x8B\xD9", "xxxx?xxxx?xxxx?xxxx?xx");
HookedFunc<__int32*, VPKInfo*, __int32*, char*> ReadFileFromVPK("filesystem_stdio.dll", "\x48\x89\x5C\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x49\x8B\xC0\x48\x8B\xDA", "xxxx?xxxx????xxxxxx");
HookedFunc<__int64, const char*> pakFunc13("rtech_game.dll", "\x48\x83\xEC\x00\x48\x8D\x15\x00\x00\x00\x00", "xxx?xxx????");
HookedFunc<__int64, unsigned int, void*> pakFunc6("rtech_game.dll", "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xDA\x8B\xF9", "xxxx?xxxx?xxxxx");
SigScanFunc<void> pakFunc1("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x8B\x01", "xxxx?xxxx?xxxx?xxxxxxxx?xx");

SigScanFunc<void> clientVMFinder("client.dll", "\x44\x8B\xC2\x48\x8B\xD1\x48\x8B\x0D\x00\x00\x00\x00", "xxxxxxxxx????");
SigScanFunc<void> serverVMFinder("server.dll", "\x48\x89\x5C\x24\x00\x55\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\xEA", "xxxx?xxxx?xxx????xxx");
SigScanFunc<void> d3d11DeviceFinder("materialsystem_dx11.dll", "\x48\x83\xEC\x00\x33\xC0\x89\x54\x24\x00\x4C\x8B\xC9\x48\x8B\x0D\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00", "xxx?xxxxx?xxxxxx????xxx?????");

HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateGeometryShader), &ID3D11DeviceVtbl::CreateGeometryShader> ID3D11Device_CreateGeometryShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreatePixelShader), &ID3D11DeviceVtbl::CreatePixelShader> ID3D11Device_CreatePixelShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateVertexShader), &ID3D11DeviceVtbl::CreateVertexShader> ID3D11Device_CreateVertexShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateComputeShader), &ID3D11DeviceVtbl::CreateComputeShader> ID3D11Device_CreateComputeShader;

HookedFunc<void, __int64> AddTextureToStats("materialsystem_dx11.dll", "\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\x48\x8D\x0D\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x0F\xB7\x43\x00", "xxxxx?xxxxxx????xx????xxx?");

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

struct CShaderGlue
{
    void** vtable;
    const char* name;
};

struct CMaterialGlue
{
    void** vtable;
    unsigned char unk[16];
    const char* name;
    unsigned char unk2[112];
    CShaderGlue* shaderGlue;
};

struct MaterialData
{
    std::string pakFile;
    std::string shaderName;
    std::vector<std::string> textures;
};

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
}

int FirstFuncOfInterestHook(IEngineClient* engineClient, const char* mapName, char a3)
{
    spdlog::get("logger")->warn("FirstFuncOfInterest: {}, {}", mapName, (int)a3);
    //int res2 = IEngineClient_FirstFuncOfInterest(engineClient, "sp_boomtown_start", 0);
    //spdlog::get("logger")->warn("res2: {}", res2);
    int res1 = IEngineClient_FirstFuncOfInterest(engineClient, mapName, a3);
    spdlog::get("logger")->warn("res1: {}", res1);
    return res1;
}

int SecondFuncOfInterest(IEngineClient* engineClient, const char* mapName)
{
    spdlog::get("logger")->warn("SecondFuncOfInterest: {}", mapName);
    return IEngineClient_SecondFuncOfInterest(engineClient, mapName);
}

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

struct model_t
{
    unsigned char fnHandle[4];
    char szName[250];
};

void V_FixSlashes(char *pname, char separator)
{
    while (*pname)
    {
        if (*pname == '\\' || *pname == '/')
        {
            *pname = separator;
        }
        pname++;
    }
}

bool isSpawningExternalMapModel = false;
std::string currentExternalMapName;


std::unordered_set<std::string> texturesToLoad;
std::unordered_set<std::string> materialsToLoad;
std::unordered_set<std::string> shadersToLoad;


std::regex textureFileRegex("^(.+\\d\\d)");
std::regex mapFromVPKRegex("client_(.+)\\.bsp");

__int64 Studio_LoadModelHook(void* modelLoader, void* model)
{
    model_t* actualModel = (model_t*)model;
    spdlog::get("logger")->debug("Studio_LoadModel: {}, {}", model, actualModel->szName);
    __int64 retval = Studio_LoadModel(modelLoader, model);
    return retval;
}

void* allocHook(__int64 a1, size_t a2, size_t a3)
{
    spdlog::get("logger")->warn("allocFunc: a2 = {}, a3 = {}", a2, a3);
    return _aligned_malloc(a2, a3);
}

void freeHook(__int64 a1, void* a2)
{
    spdlog::get("logger")->warn("freeFunc: a2 = {}", a2);
    _aligned_free(a2);
}

AllocFuncs* savedAllocFuncs = 0;
void* savedINeedThis = 0;

__int64 pakFunc3Hook(const char* src, AllocFuncs* allocFuncs, int val)
{
    __int64 retVal = pakFunc3(src, allocFuncs, val);
    spdlog::get("logger")->warn("pakFunc3: src = {}, allocFuncs = {}, val = {}, ret = {}", src, (void*)allocFuncs, val, retVal);
    savedAllocFuncs = allocFuncs;
    return retVal;
}

__int64 pakFunc9Hook(__int64 pakInstance, void* iNeedThis, void* cb)
{
    __int64 retVal = pakFunc9(pakInstance, iNeedThis, cb);
    spdlog::get("logger")->warn("pakFunc9: pak = {}, iNeedThis = {}, cb = {}, ret = {}", pakInstance, iNeedThis, cb, retVal);
    savedINeedThis = iNeedThis;
    return retVal;
}

__int64 pakFunc13Hook(const char* arg1)
{
    __int64 retVal = pakFunc13(arg1);
    spdlog::get("logger")->warn("pakFunc13: arg1 = {}, retval = {}", arg1, retVal);
    return retVal;
}

__int64 pakFunc6Hook(unsigned int arg1, void* arg2)
{
    __int64 retVal = pakFunc6(arg1, arg2);
    spdlog::get("logger")->warn("pakFunc6: arg1 = {}, arg2 = {}, retval = {}", arg1, arg2, retVal);
    return retVal;
}

struct TypeRegistration {
    char type[4];
    unsigned int unk;
    const char* niceName;
    void* func1;
    void* func2;
    void* func3;
    unsigned char unk2[56];

    bool IsValid()
    {
        return type[0] != 0;
    }
};

TypeRegistration* registrations;
void**** g_pMemAllocSingleton;

void* origFunc1;


__int64 textureFunc2(__int64 a1)
{
    //const char* name = (const char*)(a1 + 8);
    //spdlog::get("logger")->warn("texture func 2");
    return 0;
}

std::unordered_set<std::string> textureNames;
std::mutex tn;

void(*origTextureFunc1)(__int64 a1, __int64 a2, __int64 a3, __int64 a4);
void logTextureFunc1(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
    const char* name = *(const char**)(a1 + 8);
    spdlog::get("logger")->warn("texture func 1: {}", name);
    if (!isSpawningExternalMapModel)
    {
        {
            std::lock_guard<std::mutex> l(tn);
            textureNames.emplace(name);
        }
        origTextureFunc1(a1, a2, a3, a4);
    }
    else
    {
        const char* name = *(const char**)(a1 + 8);
        std::string strName(name);
        // Find the last underscore, and cut everything off after that
        /*std::size_t found = strName.find_last_of('_');
        if (found != std::string::npos)
        {
        strName = strName.substr(0, found);
        }*/

        if (texturesToLoad.find(strName) != texturesToLoad.end())
        {
            // hey, we need to actually load this!
            spdlog::get("logger")->warn("textureLoad for {} (converted to {})", name, strName);
            spdlog::get("logger")->warn("calling original textureload");
            origTextureFunc1(a1, a2, a3, a4);
        }
    }
}

void(*origTextureFunc3)(void *Dst, void *Src, void *a3);
void textureFunc3Hook(void *Dst, void *Src, void *a3)
{
    if (!isSpawningExternalMapModel)
    {
        return origTextureFunc3(Dst, Src, a3);
    }
    else
    {
        const char* name = *(const char**)((char*)Src + 8);
        std::string strName(name);
        if (texturesToLoad.find(strName) != texturesToLoad.end())
        {
            // hey, we need to actually load this!
            spdlog::get("logger")->warn("textureFunc3 for {}", strName);
            spdlog::get("logger")->warn("calling original textureFunc3");
            origTextureFunc3(Dst, Src, a3);
        }
    }
}

void doNothing()
{
    return;
}

__int64 doNothing2()
{
    return 0;
}

std::unordered_set<std::string> shaders;
std::mutex a;
std::atomic_bool shouldLogAlloc = true;


void(*origShaderFunc1)(__int64 a1, __int64 a2);
void shader_func1(__int64 a1, __int64 a2)
{
    const char* name = *(const char**)(a1);
    int type = *((int*)(a1 + 8));

    {
        std::lock_guard<std::mutex> l(a);
        spdlog::get("logger")->warn("shader func 1: {}, type = {}", (name != NULL) ? name : "NULL", type);
        if (shaders.find(name) == shaders.end()) {
            spdlog::get("logger")->warn("^^^ NEW SHADER ^^^");
            shaders.emplace(name);
            overrideShaders = true;
        }
        else
        {
            overrideShaders = false;
        }

        origShaderFunc1(a1, a2);
        overrideShaders = false;
    }
}

std::unordered_map<std::string, MaterialData> cachedMaterialData;

unsigned __int64 LoadMaterialsHook(__int64 a1, signed int* phdr, __int64 a3, __int64 a4, unsigned int a5)
{
    if (isSpawningExternalMapModel)
    {
        signed int count = phdr[52];
        spdlog::get("logger")->warn("LoadMaterials: count = {}", count);
        bool needsPakReload = false;
        for (int i = 0; i < count; i++)
        {
            signed int* v14 = (signed int *)((char *)&phdr[11 * (signed int)i] + phdr[53]);
            char *v15 = (char *)v14 + *v14;
            if (*v15 == '\\' || *v15 == '/')
                ++v15;
            char *v16 = (char *)phdr + *(signed int *)((char *)phdr + phdr[55]);
            if (*v16 == '\\' || *v16 == '/')
                ++v16;

            char path[MAX_PATH];
            strcpy_s(path, v15);
            V_FixSlashes(path, '\\');
            std::string strPath(path);

            // Check if it's in the material map
            if (cachedMaterialData.find(strPath) == cachedMaterialData.end())
            {
                spdlog::get("logger")->error("Failed to find cached material data for: {}", strPath);
                continue;
            }

            const auto& matData = cachedMaterialData[strPath];

            // Check if we have loaded this before
            if (materialsToLoad.find(strPath) == materialsToLoad.end())
            {
                // We haven't so add and mark the pak to be reloaded
                materialsToLoad.insert(strPath);

                // Add the textures
                for (const auto& tex : matData.textures)
                {
                    texturesToLoad.insert(tex);
                }

                // Add the shader
                shadersToLoad.insert(matData.shaderName);
                
                needsPakReload = true;
                spdlog::get("logger")->warn("Material {} not already loaded, will reload pak", strPath);
            }

            spdlog::get("logger")->warn("i = {}, v15 = {}, v16 = {}", i, v15, v16);
        }

        if (needsPakReload)
        {
            //origFunc1 = registrations[12].func1;
            //registrations[12].func1 = &textureLoad;

            spdlog::get("logger")->warn("loading pak for map {}", currentExternalMapName);
            std::string pakName = currentExternalMapName + ".rpak";
            __int64 ret = pakFunc3(pakName.c_str(), savedAllocFuncs, 3);
            spdlog::get("logger")->warn("my pakFunc3 for {} ret = {}", pakName.c_str(), ret);
            if (ret != -1)
            {
                __int64 ret2 = pakFunc9(ret, &doNothing, &doNothing);
                spdlog::get("logger")->warn("my pakFunc9 for {} ret = {}", pakName.c_str(), ret2);
            }
            else
            {
                spdlog::get("logger")->critical("bad things happened");
            }

            //registrations[12].func1 = origFunc1;
        }
        
    }

    return CStudioRenderContext_LoadMaterials(a1, phdr, a3, a4, a5);
}

std::mutex m;
size_t allocated = 0;
std::unordered_map<void*, __int64> allocSizes;
std::unordered_map<void*, void*> allocCallers;
std::atomic_bool inAlloc = false;


LPVOID (WINAPI*origHeapAlloc)(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ SIZE_T dwBytes);
void* pHeapAlloc;
LPVOID WINAPI HeapAllocHook(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ SIZE_T dwBytes)
{
    LPVOID mem = origHeapAlloc(hHeap, dwFlags, dwBytes);
    if (!inAlloc && shouldLogAlloc)
    {
        std::lock_guard<std::mutex> l(m);
        inAlloc = true;
        allocated += dwBytes;
        allocSizes[(void*)mem] = dwBytes;
        inAlloc = false;
    }
    return mem;
}

BOOL(WINAPI*origHeapFree)(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ LPVOID lpMem);
void* pHeapFree;
BOOL WINAPI HeapFreeHook(_In_ HANDLE hHeap, _In_ DWORD  dwFlags, _In_ LPVOID lpMem)
{
    __int64 size = 0;
    if (!inAlloc && shouldLogAlloc)
    {
        std::lock_guard<std::mutex> l(m);
        inAlloc = true;
        if (allocSizes.find(lpMem) != allocSizes.end())
        {
            size = allocSizes[lpMem];
            allocSizes[lpMem] = 0;
        }
        allocated -= size;
        inAlloc = false;
    }
    return origHeapFree(hHeap, dwFlags, lpMem);
}

struct MatFunc1
{
    void* data;
    int size;
};

std::mutex mat;
std::vector<CMaterialGlue*> g_materials;

__int64(*Material_func1Orig)(__int64, MatFunc1*);

__int64 Material_func1hook(__int64 a1, MatFunc1* a2)
{
    CMaterialGlue* glue = (CMaterialGlue*)a1;
    spdlog::get("logger")->warn("Material_func1 data = {}, size = {}, name = {}, glue = {}", a2->data, a2->size, glue->name, (void*)glue);
    {
        std::lock_guard<std::mutex> l(mat);
        g_materials.emplace_back(glue);
    }
    return Material_func1Orig(a1, a2);
}

void printAllocs()
{
    size_t largest = 0;
    size_t sum = 0;
    auto logger = spdlog::get("logger");
    for (auto it : allocSizes) {
        if (it.second != 0)
        {
            logger->info("{}: {}", it.first, it.second);
            sum += it.second;
            if (it.second > largest)
            {
                largest = it.second;
            }
        }
    }
    logger->info("largest = {}", largest);
    logger->info("sum = {}", sum);
}

void AddTextureToStats_Hook(__int64 a1)
{
    return;
}

TTF2SDK::TTF2SDK()
    : m_replacementManager("D:\\dev\\ttf2\\searchpath\\"), // TODO: make this a configuration setting. Also make sure it has a trailing slash.
      m_fileSystem("filesystem_stdio.dll", "VFileSystem017"),
      m_engineClient("engine.dll", "VEngineClient013"),
      m_engineServer("engine.dll", "VEngineServer022")
{
    m_logger = spdlog::get("logger");

    //SymInitialize(GetCurrentProcess(), NULL, TRUE);

    HMODULE mod = LoadLibraryW(L"tier0.dll");
    g_pMemAllocSingleton = (void****)GetProcAddress(mod, "g_pMemAllocSingleton");
    m_logger->warn("g_pMemAllocSingleton = {}", (void*)g_pMemAllocSingleton);
    if (g_pMemAllocSingleton == NULL)
    {
        throw std::exception("g_pMemAllocSingleton was NULL");
    }

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

    IEngineClient_FirstFuncOfInterest.Hook(m_engineClient->m_vtable, FirstFuncOfInterestHook);
    IEngineClient_SecondFuncOfInterest.Hook(m_engineClient->m_vtable, SecondFuncOfInterest);
    IEngineServer_SpewFunc.Hook(m_engineServer->m_vtable, SpewFuncHook);

    Studio_LoadModel.Hook(Studio_LoadModelHook);
    pakFunc3.Hook(pakFunc3Hook);
    pakFunc9.Hook(pakFunc9Hook);
    pakFunc13.Hook(pakFunc13Hook);
    pakFunc6.Hook(pakFunc6Hook);
    CStudioRenderContext_LoadMaterials.Hook(LoadMaterialsHook);

    unsigned char* addTextureToStatsBase = (unsigned char*)AddTextureToStats.GetFuncPtr();
    {
        TempReadWrite rw(addTextureToStatsBase);
        addTextureToStatsBase += 0x33;
        //addTextureToStatsBase[0] = 0x90;
        //addTextureToStatsBase[1] = 0x90;
        //addTextureToStatsBase[2] = 0x90;
        //addTextureToStatsBase[3] = 0x90;
    }

    //AddTextureToStats.Hook(AddTextureToStats_Hook);

    __int64 base = (__int64)pakFunc1.GetFuncPtr() + 33;
    __int32 offsetAgain = *((__int32*)(base - 4));
    registrations = (TypeRegistration*)(base + offsetAgain);
    m_logger->warn("registrations = {}", (void*)registrations);
    origShaderFunc1 = (decltype(origShaderFunc1))registrations[13].func1;
    registrations[13].func1 = shader_func1;

    Material_func1Orig = (decltype(Material_func1Orig))registrations[4].func1;
    registrations[4].func1 = Material_func1hook;

    origTextureFunc1 = (decltype(origTextureFunc1))registrations[12].func1;
    registrations[12].func1 = logTextureFunc1;

    origTextureFunc3 = (decltype(origTextureFunc3))registrations[12].func3;
    registrations[12].func3 = textureFunc3Hook;

    base_print.Hook(WRAPPED_MEMBER(BasePrintHook<CONTEXT_CLIENT>), WRAPPED_MEMBER(BasePrintHook<CONTEXT_SERVER>));
    sqstd_compiler_error.Hook(WRAPPED_MEMBER(CompilerErrorHook<CONTEXT_CLIENT>), WRAPPED_MEMBER(CompilerErrorHook<CONTEXT_SERVER>));

    _Host_RunFrame.Hook(WRAPPED_MEMBER(RunFrameHook));

    pHeapAlloc = Util::ResolveLibraryFunction("kernel32.dll", "HeapAlloc");
    pHeapFree = Util::ResolveLibraryFunction("kernel32.dll", "HeapFree");

    // Get pointer to d3d device
    funcBase = (char*)d3d11DeviceFinder.GetFuncPtr();
    offset = *(int*)(funcBase + 16);
    m_ppD3D11Device = (ID3D11Device**)(funcBase + 20 + offset);

    m_logger->debug("m_ppD3D11Device = {}", (void*)m_ppD3D11Device);
    m_logger->debug("pD3D11Device = {}", (void*)*m_ppD3D11Device);
    m_logger->debug("queryinterface = {}", offsetof(ID3D11DeviceVtbl, QueryInterface));
    m_logger->debug("address of the func = {}", (void*)(((char*)(*m_ppD3D11Device)->lpVtbl) + offsetof(ID3D11DeviceVtbl, QueryInterface)));

    ID3D11Device_CreateGeometryShader.Hook((*m_ppD3D11Device)->lpVtbl, CreateGeometryShader_Hook);
    ID3D11Device_CreatePixelShader.Hook((*m_ppD3D11Device)->lpVtbl, CreatePixelShader_Hook);
    ID3D11Device_CreateComputeShader.Hook((*m_ppD3D11Device)->lpVtbl, CreateComputeShader_Hook);
    ID3D11Device_CreateVertexShader.Hook((*m_ppD3D11Device)->lpVtbl, CreateVertexShader_Hook);

    compileShaders();
}

void hookHeap()
{
    auto m_logger = spdlog::get("logger");

    MH_STATUS status = MH_CreateHookEx(pHeapAlloc, HeapAllocHook, &origHeapAlloc);
    if (status != MH_OK)
    {
        m_logger->critical("MH_CreateHook returned {}", status);
        throw std::exception("Failed to hook function");
    }

    status = MH_EnableHook(pHeapAlloc);
    if (status != MH_OK)
    {
        m_logger->critical("MH_EnableHook returned {}", status);
        throw std::exception("Failed to enable hook");
    }

    status = MH_CreateHookEx(pHeapFree, HeapFreeHook, &origHeapFree);
    if (status != MH_OK)
    {
        m_logger->critical("MH_CreateHook returned {}", status);
        throw std::exception("Failed to hook function");
    }

    status = MH_EnableHook(pHeapFree);
    if (status != MH_OK)
    {
        m_logger->critical("MH_EnableHook returned {}", status);
        throw std::exception("Failed to enable hook");
    }
}

void unhookHeap()
{
    MH_RemoveHook(pHeapAlloc);
    MH_RemoveHook(pHeapFree);
}

void addTextureIfNeeded(MaterialData& data, const std::string& matName, const char* ext)
{
    std::string texName = matName + ext;

    if (textureNames.find(texName) != textureNames.end())
    {
        data.textures.emplace_back(texName);
    }
}

void resolveMaterials(const std::string& pakName)
{
    for (auto mat : g_materials)
    {
        MaterialData data;
        data.pakFile = pakName;
        data.shaderName = mat->shaderGlue->name;
        
        std::string texName = mat->name;
        std::smatch m;
        std::regex_search(texName, m, textureFileRegex);
        if (!m.empty())
        {
            texName = m[1];
        }

        addTextureIfNeeded(data, texName, "_col");
        addTextureIfNeeded(data, texName, "_nml");
        addTextureIfNeeded(data, texName, "_spc");
        addTextureIfNeeded(data, texName, "_exp");
        addTextureIfNeeded(data, texName, "_ilm");
        addTextureIfNeeded(data, texName, "_glw");
        addTextureIfNeeded(data, texName, "_lim");
        addTextureIfNeeded(data, texName, "_gls");
        addTextureIfNeeded(data, texName, "_bm");
        addTextureIfNeeded(data, texName, "_opa");
        addTextureIfNeeded(data, texName, "_cav");
        addTextureIfNeeded(data, texName, "_ao");

        cachedMaterialData[mat->name] = data;
    }
}

void printCachedMatData()
{
    std::string pakFile;
    std::string shaderName;
    std::vector<std::string> textures;

    auto logger = spdlog::get("logger");
    for (auto it : cachedMaterialData)
    {
        std::string textures;
        for (auto name : it.second.textures)
        {
            textures += name + " ";
        }
        logger->info("{}", it.first);
        logger->info("\tpak = {}", it.second.pakFile);
        logger->info("\tshader = {}", it.second.shaderName);
        logger->info("\ttextures = {}", textures);
    }
}

void preload(const char* name)
{
    textureNames.clear();
    g_materials.clear();
    //inLoad = true;

    spdlog::get("logger")->warn("loading {}", name);
    __int64 ret = pakFunc3(name, savedAllocFuncs, 3);
    spdlog::get("logger")->warn("my pakFunc3 for {} ret = {}", name, ret);
    if (ret != -1)
    {
        __int64 ret2 = pakFunc9(ret, &doNothing, &doNothing);
        spdlog::get("logger")->warn("my pakFunc9 for {} ret = {}", name, ret2);
        if (ret2 == 1)
        {
            resolveMaterials(name);
            __int64 ret3 = pakFunc6(ret, &doNothing);
            spdlog::get("logger")->warn("my pakFunc6 for {} ret = {}", name, ret3);
        }
    }

    //inLoad = false;
}

void printRegs()
{
    for (int i = 0; i < 16; i++) {
        TypeRegistration* reg = &registrations[i];
        if (reg->IsValid())
        {
            spdlog::get("logger")->warn("reg {}: {}{}{}{} - {}", i, reg->type[0], reg->type[1], reg->type[2], reg->type[3], reg->niceName);
        }
        else
        {
            spdlog::get("logger")->warn("reg {}: EMPTY", i);
        }
    }
}

__int64(*origFreeFunc)(void* allocator, void* mem);
__int64(*origAllocFunc)(void* allocator, __int64 size);

__int64 memFreeHook(void* allocator, void* mem)
{
    __int64 size = 0;
    {
        std::lock_guard<std::mutex> l(m);
        if (allocSizes.find(mem) != allocSizes.end())
        {
            size = allocSizes[mem];
        }
        allocated -= size;
    }

    spdlog::get("logger")->warn("freeing {} at {}", size, mem);
    return origFreeFunc(allocator, mem);
}

__int64 memAllocAllocHook(void* allocator, __int64 size)
{
    __int64 mem = origAllocFunc(allocator, size);
    spdlog::get("logger")->warn("allocated {} at {}", size, (void*)mem);
    {
        std::lock_guard<std::mutex> l(m);
        allocated += size;
        allocSizes[(void*)mem] = size;
    }
    return mem;
}

void TTF2SDK::RunFrameHook(double absTime, float frameTime)
{
    static bool searchPathAdded = false;
    if (!searchPathAdded)
    {
        m_logger->info("Adding mod search path: {}", m_replacementManager.GetSearchPath());
        IFileSystem_AddSearchPath(m_fileSystem, m_replacementManager.GetSearchPath().c_str(), "GAME", PATH_ADD_TO_HEAD);
        searchPathAdded = true;
    }

    if (m_shouldRunServerCode)
    {
        auto v = GetServerSQVM();
        if (v != nullptr)
        {
            std::string code = GetServerCode();
            if (code == "load")
            {
                void* origFuncs1[16];
                void* origFuncs2[16];

                //for (int i = 0; i < 16; i++)
                //{
                //    if (i == 13 || i == 4 || i == 6 || i == 7 || i == 8 || i == 12)
                //    {
                //        origFuncs1[i] = registrations[i].func1;
                //        origFuncs2[i] = registrations[i].func2;
                //        if (registrations[i].func1 != NULL)
                //        {
                //            registrations[i].func1 = doNothing;
                //        }
                //        
                //        if (registrations[i].func2 != NULL)
                //        {
                //            registrations[i].func2 = textureFunc2;
                //        }
                //    }
                //}

                origFunc1 = registrations[12].func1;
                void* origFunc2 = registrations[12].func2;
                origAllocFunc = (decltype(origAllocFunc))*((**g_pMemAllocSingleton) + 1);
                origFreeFunc = (decltype(origFreeFunc))*((**g_pMemAllocSingleton) + 5);
                {
                    TempReadWrite rw(**g_pMemAllocSingleton);
                    // *((**g_pMemAllocSingleton) + 1) = &memAllocAllocHook;
                     //*((**g_pMemAllocSingleton) + 5) = &memFreeHook;
                }

                // m_logger->warn("original func1 = {}", origFunc1);
                 //registrations[12].func1 = &doNothing2;
                 //registrations[12].func2 = &doNothing2;
                void* origAlloc = savedAllocFuncs->allocFunc;
                void* origFree = savedAllocFuncs->freeFunc;
                //savedAllocFuncs->allocFunc = allocHook;
                //savedAllocFuncs->freeFunc = freeHook;
                //hookHeap();
                cachedMaterialData.clear();
                preload("sp_boomtown_start.rpak");
                preload("sp_boomtown_end.rpak");
                preload("sp_crashsite.rpak");
                preload("sp_sewers1.rpak");
                preload("sp_skyway_v1.rpak");
                preload("sp_timeshift_spoke02.rpak");
                preload("sp_tday.rpak");
                preload("sp_s2s.rpak");
                preload("sp_beacon_spoke0.rpak");
                //unhookHeap();
                m_logger->warn("allocated = {}", allocated);
                //savedAllocFuncs->allocFunc = (decltype(savedAllocFuncs->allocFunc))origAlloc;
                //savedAllocFuncs->freeFunc = (decltype(savedAllocFuncs->freeFunc))origFree;
                {
                    TempReadWrite rw(**g_pMemAllocSingleton);
                    //*((**g_pMemAllocSingleton) + 1) = origAllocFunc;
                   // *((**g_pMemAllocSingleton) + 5) = origFreeFunc;
                }
                //registrations[12].func1 = origFunc1;
                //registrations[12].func2 = origFunc2;

                //for (int i = 0; i < 16; i++)
                //{
                //    if (i == 13 || i == 4 || i == 6 || i == 7 || i == 8 || i == 12)
                //    {
                //        registrations[i].func1 = origFuncs1[i];
                //        registrations[i].func2 = origFuncs2[i];
                //    }
                //}
            }
            else if (code == "printregs")
            {
                printRegs();
            }
            else if (code == "printallocs")
            {
                printAllocs();
            }
            else if (code == "spawnmodelmode")
            {
                isSpawningExternalMapModel = !isSpawningExternalMapModel;
                m_logger->warn("isSpawningExternalMapModel = {}", isSpawningExternalMapModel);
            }
            else if (code == "printshaders")
            {
                for (auto it : shaders)
                {
                    m_logger->warn("{}", it);
                }
                m_logger->warn("num shaders = {}", shaders.size());
            }
            else if (code == "printmats")
            {
                printCachedMatData();
            }
            else if (code == "compileshaders")
            {
                compileShaders();
            }
            else if (code == "printtoload")
            {
                m_logger->info("Textures:");
                for (auto t : texturesToLoad)
                {
                    m_logger->info("\t{}", t);
                }

                m_logger->info("Materials:");
                for (auto t : materialsToLoad)
                {
                    m_logger->info("\t{}", t);
                }

                m_logger->info("Shaders:");
                for (auto t : shadersToLoad)
                {
                    m_logger->info("\t{}", t);
                }
            }
            else
            {
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
    // If the path is one of our replacements, we will not allow the cache to respond
    if (m_replacementManager.ShouldReplaceFile(filename))
    {
        m_logger->info("ReadFileFromVPK: blocking response for {} from {}", filename, vpkInfo->path);
        *b = -1;
        return b;
    }

    __int32* result = ReadFileFromVPK(vpkInfo, b, filename);
    m_logger->debug("ReadFileFromVPK: vpk = {}, file = {}, result = {}", vpkInfo->path, filename, *b);

    if (isSpawningExternalMapModel)
    {
        std::smatch m;
        std::string strPath(vpkInfo->path);
        std::regex_search(strPath, m, mapFromVPKRegex);
        if (!m.empty())
        {
            currentExternalMapName = m[1];
        }

        m_logger->warn("Set currentExternalMapName = {}", currentExternalMapName);
    }

    return result;
}

unsigned int* TTF2SDK::MountVPKHook(IFileSystem* fileSystem, const char* vpkPath)
{
    m_logger->debug("IFileSystem::MountVPK: vpkPath = {}", vpkPath);
    unsigned int* res = IFileSystem_MountVPK(fileSystem, vpkPath);
    if (strcmp(vpkPath, "vpk/client_sp_boomtown_start.bsp") != 0)
    {
        unsigned int* res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_boomtown_start.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
        res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_crashsite.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
        res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_timeshift_spoke02.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
        res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_skyway_v1.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
        res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_tday.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
        res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_sewers1.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
        res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_s2s.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
        res2 = IFileSystem_MountVPK(fileSystem, "vpk/client_sp_beacon_spoke0.bsp"); // TODO: Remove this
        m_logger->debug("injected res = {}", (void*)res2);
    }
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
