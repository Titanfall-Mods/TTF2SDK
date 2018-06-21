#include "stdafx.h"

const int PAK_CACHE_FILE_VERSION = 2;

PakManager& PakMan()
{
    return SDK().GetPakManager();
}

void* PakAlloc(int64_t a1, size_t size, size_t align)
{
    return _aligned_malloc(size, align);
}

void PakFree(int64_t a1, void* ptr)
{
    _aligned_free(ptr);
}

PakAllocFuncs g_SDKAllocFuncs = {
    PakAlloc,
    PakFree
};

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&PakManager::##name), &PakManager::##name, decltype(&PakMan), &PakMan>::Call

SigScanFunc<void> PakRefFinder("engine.dll", "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x83\x3D\x00\x00\x00\x00\x00\x44\x8B\x0D\x00\x00\x00\x00", "xxxx?xxxx?xxxx?xx?????xxx????");
HookedFunc<bool, const char*> LoadMapPak("engine.dll", "\x48\x81\xEC\x00\x00\x00\x00\x48\x8D\x54\x24\x00\x41\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x4C\x24\x00", "xxx????xxxx?xx????x????xxxx?");
SigScanFunc<void> PakFunc1("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x8B\x01", "xxxx?xxxx?xxxx?xxxxxxxx?xx");
HookedFunc<int32_t, const char*, PakAllocFuncs*, int> PakFunc3("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF1\x48\x8D\x0D\x00\x00\x00\x00", "xxxx?xxxx?xxxx?xxxxxx????");
HookedFunc<int64_t, int32_t, void*> PakFunc6("rtech_game.dll", "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xDA\x8B\xF9", "xxxx?xxxx?xxxxx");
HookedFunc<int64_t, int32_t, void*, void*> PakFunc9("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x8B\xD9", "xxxx?xxxx?xxxx?xxxx?xx");
HookedFunc<int64_t, const char*> PakFunc13("rtech_game.dll", "\x48\x83\xEC\x00\x48\x8D\x15\x00\x00\x00\x00", "xxx?xxx????");

HookedFunc<int64_t, void*, model_t*> Studio_LoadModel("engine.dll", "\x48\x89\x6C\x24\x00\x41\x56\x48\x83\xEC\x00\x83\x8A\x00\x00\x00\x00\x00", "xxxx?xxxxx?xx?????");
HookedFunc<uint64_t, int64_t, int32_t*, int64_t, studioloddata_t*, uint32_t> CStudioRenderContext_LoadMaterials("studiorender.dll", "\x40\x53\x55\x57", "xxxx");
HookedFunc<int64_t, int*, int32_t*, int64_t, unsigned int> LoadMaterials_SubFunc("studiorender.dll", "\x48\x8B\xC4\x44\x89\x48\x00\x4C\x89\x40\x00\x48\x89\x48\x00", "xxxxxx?xxx?xxx?");
SigScanFunc<int64_t, void*, void*> CModelLoader_UnloadModel("engine.dll", "\x48\x89\x5C\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xDA\x8B\x92\x00\x00\x00\x00", "xxxx?xxxx????xxxxx????");
HookedFunc<int64_t, void*, const char*> CBaseEntity_SetModel("server.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x33\xFF\x48\x8B\xDA", "xxxx?xxxx?xxxx?xxxx?xxxxx");

HookedVTableFunc<decltype(&IVEngineServer::VTable::PrecacheModel), &IVEngineServer::VTable::PrecacheModel> IVEngineServer_PrecacheModel;
SigScanFunc<void> CServerFinder("engine.dll", "\x48\x83\xEC\x00\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x85\xC0", "xxx?xxx????x????xx");
SigScanFunc<void> CClientStateFinder("engine.dll", "\x41\x83\xF8\x00\x7C\x00\x53", "xxx?x?x");

HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateGeometryShader), &ID3D11DeviceVtbl::CreateGeometryShader> ID3D11Device_CreateGeometryShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreatePixelShader), &ID3D11DeviceVtbl::CreatePixelShader> ID3D11Device_CreatePixelShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateVertexShader), &ID3D11DeviceVtbl::CreateVertexShader> ID3D11Device_CreateVertexShader;
HookedVTableFunc<decltype(&ID3D11DeviceVtbl::CreateComputeShader), &ID3D11DeviceVtbl::CreateComputeShader> ID3D11Device_CreateComputeShader;

const int MAT_REG_INDEX = 4;
const int TEX_REG_INDEX = 12;
const int SHADER_REG_INDEX = 13;

PakManager::PakManager(
    ConCommandManager& conCommandManager, 
    SourceInterface<IVEngineServer> engineServer,
    SquirrelManager& squirrelManager,
    ID3D11Device** ppD3DDevice
) :
    m_modelInfo("engine.dll", "VModelInfoServer002")
{
    m_logger = spdlog::get("logger");
    m_state = PAK_STATE_NONE;

    CreateDummyShaders(ppD3DDevice);

    int64_t base = (int64_t)PakFunc1.GetFuncPtr() + 33;
    int32_t offset = *((int32_t*)(base - 4));
    m_typeRegistrations = (TypeRegistration*)(base + offset);
    SPDLOG_DEBUG(m_logger, "Type Registrations: {}", (void*)m_typeRegistrations);

    // Check that the registrations we care about are where we expect
    if (!m_typeRegistrations[MAT_REG_INDEX].IsTypeEqual("matl"))
    {
        throw std::exception("registration 4 is not matl");
    }

    if (!m_typeRegistrations[TEX_REG_INDEX].IsTypeEqual("txtr"))
    {
        throw std::exception("registration 12 is not txtr");
    }

    if (!m_typeRegistrations[SHADER_REG_INDEX].IsTypeEqual("shdr"))
    {
        throw std::exception("registration 13 is not shdr");
    }

    // Get the pak refs array from the engine
    base = (int64_t)PakRefFinder.GetFuncPtr() + 58;
    offset = *((int32_t*)(base - 5));
    m_pakRefs = (int32_t*)(base + offset);
    SPDLOG_DEBUG(m_logger, "Pak refs: {}", (void*)m_pakRefs);

    // Get the reference to CServer
    base = (int64_t)CServerFinder.GetFuncPtr() + 11;
    offset = *((int32_t*)(base - 4));
    m_gameServer = (CGameServer*)(base + offset);
    SPDLOG_DEBUG(m_logger, "Game Server: {}", (void*)m_gameServer);

    // Get the reference to CClientState
    base = (int64_t)CClientStateFinder.GetFuncPtr() + 19;
    offset = *((int32_t*)(base - 4));
    tClientStateFunc f = (tClientStateFunc)(base + offset);
    m_clientState = f();
    SPDLOG_DEBUG(m_logger, "Client State: {}", (void*)m_clientState);

    m_matFunc1.Hook(&m_typeRegistrations[MAT_REG_INDEX], 0, WRAPPED_MEMBER(MaterialFunc1Hook));
    m_texFunc1.Hook(&m_typeRegistrations[TEX_REG_INDEX], 0, WRAPPED_MEMBER(TextureFunc1Hook));
    m_texFunc2.Hook(&m_typeRegistrations[TEX_REG_INDEX], 1, WRAPPED_MEMBER(TextureFunc2Hook));
    m_texFunc3.Hook(&m_typeRegistrations[TEX_REG_INDEX], 2, WRAPPED_MEMBER(TextureFunc3Hook));
    m_shaderFunc1.Hook(&m_typeRegistrations[SHADER_REG_INDEX], 0, WRAPPED_MEMBER(ShaderFunc1Hook));
    m_shaderFunc2.Hook(&m_typeRegistrations[SHADER_REG_INDEX], 1, WRAPPED_MEMBER(ShaderFunc2Hook));

    PakFunc6.Hook(WRAPPED_MEMBER(PakFunc6Hook));
#ifdef _DEBUG
    PakFunc3.Hook(WRAPPED_MEMBER(PakFunc3Hook));
    PakFunc9.Hook(WRAPPED_MEMBER(PakFunc9Hook));
    PakFunc13.Hook(WRAPPED_MEMBER(PakFunc13Hook));
#endif

    Studio_LoadModel.Hook(WRAPPED_MEMBER(Studio_LoadModelHook));
    CStudioRenderContext_LoadMaterials.Hook(WRAPPED_MEMBER(LoadMaterialsHook));
    LoadMaterials_SubFunc.Hook(WRAPPED_MEMBER(LoadMaterials_SubFunc_Hook));
    LoadMapPak.Hook(WRAPPED_MEMBER(LoadMapPakHook));
    CBaseEntity_SetModel.Hook(WRAPPED_MEMBER(SetModelHook));

    IVEngineServer_PrecacheModel.Hook(engineServer->m_vtable, WRAPPED_MEMBER(PrecacheModelHook));

    ID3D11Device_CreateGeometryShader.Hook((*ppD3DDevice)->lpVtbl, WRAPPED_MEMBER(CreateGeometryShader_Hook));
    ID3D11Device_CreatePixelShader.Hook((*ppD3DDevice)->lpVtbl, WRAPPED_MEMBER(CreatePixelShader_Hook));
    ID3D11Device_CreateComputeShader.Hook((*ppD3DDevice)->lpVtbl, WRAPPED_MEMBER(CreateComputeShader_Hook));
    ID3D11Device_CreateVertexShader.Hook((*ppD3DDevice)->lpVtbl, WRAPPED_MEMBER(CreateVertexShader_Hook));

    conCommandManager.RegisterCommand("print_registrations", WRAPPED_MEMBER(PrintRegistrations), "Print type registrations for pak system", 0);
    conCommandManager.RegisterCommand("print_pak_refs", WRAPPED_MEMBER(PrintPakRefs), "Print engine pak references", 0);
    conCommandManager.RegisterCommand("print_cached_materials", WRAPPED_MEMBER(PrintCachedMaterialData), "Print cached material data", 0);
    conCommandManager.RegisterCommand("print_external_models", WRAPPED_MEMBER(PrintExternalModels), "Prints all the models loaded from external VPKs", 0);
    conCommandManager.RegisterCommand("print_current_level_pak", WRAPPED_MEMBER(PrintCurrentLevelPak), "Prints current level pak file", 0);
    // TODO: Command to print stuff we want to load

    squirrelManager.AddFuncRegistration(CONTEXT_SERVER, "void", "EnableExternalSpawnMode", "", "", WRAPPED_MEMBER(EnableExternalSpawnMode));
    squirrelManager.AddFuncRegistration(CONTEXT_SERVER, "void", "DisableExternalSpawnMode", "", "", WRAPPED_MEMBER(DisableExternalSpawnMode));
}

void PakManager::PrintRegistrations(const CCommand& args)
{
    for (int i = 0; i < 16; i++)
    {
        TypeRegistration* reg = &m_typeRegistrations[i];
        if (reg->IsValid())
        {
            m_logger->info("Reg {}: {}{}{}{} - {}", i, reg->type[0], reg->type[1], reg->type[2], reg->type[3], reg->niceName);
        }
        else
        {
            m_logger->info("Reg {}: EMPTY", i);
        }
    }
}

void PakManager::PrintPakRefs(const CCommand& args)
{
    for (int i = 0; i < 4; i++)
    {
        m_logger->info("Ref {}: {}", i, m_pakRefs[i]);
    }
}

void PakManager::PrintCachedMaterialData(const CCommand& args)
{
    for (auto& it : m_cachedMaterialData)
    {
        m_logger->info("{}", it.first);
        m_logger->info("\tpakFiles = {}", Util::ConcatStrings(it.second.pakFiles, ", "));
        m_logger->info("\tshaderNames = {}", Util::ConcatStrings(it.second.shaderNames, ", "));
        m_logger->info("\ttextures = {}", Util::ConcatStrings(it.second.textures, ", "));
    }
}

void PakManager::PrintExternalModels(const CCommand& args)
{
    for (const auto& it : m_loadedExternalModels)
    {
        m_logger->info("{}", it.first);
        for (model_t* model : it.second)
        {
            m_logger->info("\t{}", model->szName);
        }
    }
}

void PakManager::PrintCurrentLevelPak(const CCommand& args)
{
    m_logger->info(m_currentLevelPak);
}

SQInteger PakManager::EnableExternalSpawnMode(HSQUIRRELVM v)
{
    if (m_state != PAK_STATE_NONE && m_state != PAK_STATE_SPAWN_EXTERNAL)
    {
        m_logger->error("Cannot change spawn mode now - must be in state PAK_STATE_NONE or PAK_STATE_SPAWN_EXTERNAL");
        return 0;
    }

    m_logger->info("Pak state set to PAK_STATE_SPAWN_EXTERNAL");
    m_state = PAK_STATE_SPAWN_EXTERNAL;
    return 0;
}

SQInteger PakManager::DisableExternalSpawnMode(HSQUIRRELVM v)
{
    if (m_state != PAK_STATE_NONE && m_state != PAK_STATE_SPAWN_EXTERNAL)
    {
        m_logger->error("Cannot change spawn mode now - must be in state PAK_STATE_NONE or PAK_STATE_SPAWN_EXTERNAL");
        return 0;
    }

    m_logger->info("Pak state set to PAK_STATE_NONE");
    m_state = PAK_STATE_NONE;
    return 0;
}

std::string PakManager::GetGameBuild()
{
    std::ifstream buildTxt("build.txt");
    if (!buildTxt.good())
    {
        m_logger->error("Failed to read build.txt from game folder, cannot load pak cache");
        return false;
    }

    return Util::ReadFileToString(buildTxt);
}

void doNothing()
{
    return;
}

void PakManager::AddTextureIfExists(CachedMaterialData& data, const std::string& matName, const char* ext)
{
    std::string texName = matName + ext;
    if (m_tempLoadedTextures.find(texName) != m_tempLoadedTextures.end())
    {
        if (std::find(data.textures.begin(), data.textures.end(), texName) == data.textures.end())
        {
            data.textures.emplace_back(texName);
        }
    }
}

void PakManager::ResolveMaterials(const std::string& pakName)
{
    static std::regex textureFileRegex("^(.+\\d\\d)");

    for (auto mat : m_tempLoadedMaterials)
    {
        CachedMaterialData& data = m_cachedMaterialData[mat->name];

        if (std::find(data.pakFiles.begin(), data.pakFiles.end(), pakName) == data.pakFiles.end())
        {
            data.pakFiles.emplace_back(pakName);
        }
        
        if (std::find(data.shaderNames.begin(), data.shaderNames.end(), mat->shaderGlue->name) == data.shaderNames.end())
        {
            data.shaderNames.emplace_back(mat->shaderGlue->name);
        }
        
        std::string texName = mat->name;
        std::smatch m;
        std::regex_search(texName, m, textureFileRegex);
        if (!m.empty())
        {
            texName = m[1];
        }

        AddTextureIfExists(data, texName, "_col");
        AddTextureIfExists(data, texName, "_nml");
        AddTextureIfExists(data, texName, "_spc");
        AddTextureIfExists(data, texName, "_exp");
        AddTextureIfExists(data, texName, "_ilm");
        AddTextureIfExists(data, texName, "_glw");
        AddTextureIfExists(data, texName, "_lim");
        AddTextureIfExists(data, texName, "_gls");
        AddTextureIfExists(data, texName, "_bm");
        AddTextureIfExists(data, texName, "_opa");
        AddTextureIfExists(data, texName, "_cav");
        AddTextureIfExists(data, texName, "_ao");
    }
}

void PakManager::PreloadPak(const char* name)
{
    if (m_state != PAK_STATE_NONE)
    {
        m_logger->error("Cannot preload paks unless in PAK_STATE_NONE");
        return;
    }

    // Clear previous preload data
    m_tempLoadedShaders.clear();
    m_tempLoadedTextures.clear();
    m_tempLoadedMaterials.clear();

    m_logger->info("Preloading pak: {}", name);

    // Initialise
    int32_t pakRef = PakFunc3(name, &g_SDKAllocFuncs, 3);
    if (pakRef == -1)
    {
        m_logger->error("Failed to preload pak {}, PakFunc3 failed", name);
        return;
    }

    SPDLOG_DEBUG(m_logger, "PakFunc3 completed (ref = {})", pakRef);

    // Set state to preload
    m_state = PAK_STATE_PRELOAD;

    // Load
    int64_t result = PakFunc9(pakRef, &doNothing, &doNothing);
    m_state = PAK_STATE_NONE;
    if (result != 1)
    {
        m_logger->error("Failed to preload pak {}, PakFunc9 failed ({})", name, result);
        return;
    }

    SPDLOG_DEBUG(m_logger, "PakFunc9 completed (result = {})", result);

    // Build data structure with all loaded assets
    ResolveMaterials(name);

    // Free pak
    m_state = PAK_STATE_PRELOAD;
    PakFunc6(pakRef, &doNothing);
    m_state = PAK_STATE_NONE;

    m_logger->info("Finished preloading pak: {}", name);
}

bool PakManager::LoadCacheFile()
{
    // Check if pak cache exists
    std::ifstream cacheFile(SDK().GetFSManager().GetBasePath() / "pakcache.dat", std::ios::binary);
    if (!cacheFile.good())
    {
        return false;
    }

    m_logger->info("Loading pak cache from file...");

    ttf2sdk::PakCache pbCache;
    if (!pbCache.ParseFromIstream(&cacheFile))
    {
        m_logger->error("Failed to load pak cache from file");
        return false;
    }

    // Check that the cache is the same as our expected file version
    if (pbCache.cacheversion() != PAK_CACHE_FILE_VERSION)
    {
        m_logger->warn("Pak cache version mismatch (file = {}, current = {})", pbCache.cacheversion(), PAK_CACHE_FILE_VERSION);
        return false;
    }

    // Check if the cache's build is the same as the current build
    // Read the contents of build.txt in the game folder
    std::string build = GetGameBuild();
    if (pbCache.gamebuild() != build)
    {
        m_logger->warn("Pak cache game build mismatch (file = {}, current = {})", pbCache.gamebuild(), build);
        return false;
    }

    for (const auto& mat : pbCache.materials())
    {
        CachedMaterialData& data = m_cachedMaterialData[mat.name()];
        for (const auto& pakFile : mat.pakfiles())
        {
            data.pakFiles.emplace_back(pakFile);
        }

        for (const auto& shaderName : mat.shadernames())
        {
            data.shaderNames.emplace_back(shaderName);
        }

        for (const auto& texture : mat.textures())
        {
            data.textures.emplace_back(texture);
        }
    }

    m_logger->info("Successfully loaded pak cache from file");
    return true;
}

void PakManager::PreloadAllPaks()
{
    m_cachedMaterialData.clear();

    // Check if pakcache exists and load if so
    if (LoadCacheFile())
    {
        return;
    }

    FileSystemManager& fsManager = SDK().GetFSManager();
    const auto& mapNames = fsManager.GetMapNames();
    std::shared_ptr<IFrameTask> task = std::make_shared<Preloader>(mapNames);
    SDK().AddFrameTask(std::move(task));
}

void PakManager::SortCachedMaterialData()
{
    for (auto& entry : m_cachedMaterialData)
    {
        CachedMaterialData& data = entry.second;
        std::sort(data.pakFiles.begin(), data.pakFiles.end());
        std::sort(data.shaderNames.begin(), data.shaderNames.end());
        std::sort(data.textures.begin(), data.textures.end());
    }
}

void PakManager::ReloadExternalPak(const std::string& pakFile, std::unordered_set<std::string>& newMaterialsToLoad, std::unordered_set<std::string>& newTexturesToLoad, std::unordered_set<std::string>& newShadersToLoad)
{
    std::vector<std::string> modelsToReload;
    if (IsExternalPakLoaded(pakFile))
    {
        m_logger->info("Need to unload pak {} to load additional assets", pakFile);

        // Unload the actual pak
        PakState origState = m_state;
        m_state = PAK_STATE_UNLOAD_EXTERNAL;
        int32_t pakRef = m_loadedExternalPaks[pakFile];
        PakFunc6(pakRef, &doNothing);
        m_loadedExternalPaks.erase(pakFile);
        m_logger->info("Pak {} unloaded", pakFile);
        m_state = origState;
    }

    m_logger->info("Loading {} materials, {} textures, {} shaders from {}", newMaterialsToLoad.size(), newTexturesToLoad.size(), newShadersToLoad.size(), pakFile);
   
    // Update the actual lists with new items to load
    for (auto& mat : newMaterialsToLoad)
    {
        m_materialsToLoad.emplace(std::move(mat));
    }

    for (auto& tex : newTexturesToLoad)
    {
        m_texturesToLoad.emplace(std::move(tex));
    }

    for (auto& shader : newShadersToLoad)
    {
        m_shadersToLoad.emplace(std::move(shader));
    }

    // Load the pak
    LoadExternalPak(pakFile);

    // Reload materials for all the models we had originally loaded
    for (model_t* mdl : m_loadedExternalModels[pakFile])
    {
        if (m_loadMaterialArgs.find(mdl) == m_loadMaterialArgs.end())
        {
            m_logger->error("Failed to get material args for model {}, {}", (void*)mdl, mdl->szName);
            continue;
        }

        const LoadMaterialArgs& args = m_loadMaterialArgs[mdl];
        SPDLOG_TRACE(m_logger, "Loading materials for {}", mdl->szName);
         
        {
            std::vector<int> materialFlags(args.lodData->pMaterialFlags, args.lodData->pMaterialFlags + args.lodData->numMaterials);
            m_savedMaterialFlags.insert_or_assign(args.phdr, std::move(materialFlags));
        }

        // TODO: This leaks some memory in lodData(ppMaterials and flags).Will need to manually free these.
        PakState origState = m_state;
        m_state = PAK_STATE_RESETTING_MATERIALS;
        CStudioRenderContext_LoadMaterials((int64_t)m_studioRenderContext, args.phdr, args.a3, args.lodData, args.a5);
        m_state = origState;
    }
}

void PakManager::LoadExternalPak(const std::string& pakFile)
{
    if (m_state != PAK_STATE_SPAWN_EXTERNAL)
    {
        m_logger->error("Cannot load external pak unless in state PAK_STATE_SPAWN_EXTERNAL");
        return;
    }

    if (IsExternalPakLoaded(pakFile))
    {
        m_logger->error("Cannot load external pak {}, already loaded", pakFile);
        return;
    }

    m_logger->info("Loading external pak: {}", pakFile);
    int32_t pakRef = PakFunc3(pakFile.c_str(), &g_SDKAllocFuncs, 3);
    if (pakRef == -1)
    {
        m_logger->error("Failed to load pak, PakFunc3 failed");
        return;
    }

    int64_t result = PakFunc9(pakRef, &doNothing, &doNothing);
    if (result != 1)
    {
        m_logger->error("Failed to load pak, PakFunc9 failed ({})", result);
        return;
    }
    
    m_logger->info("Finished loading {}", pakFile);
    m_loadedExternalPaks.emplace(pakFile, pakRef);
}

bool PakManager::IsExternalPakLoaded(const std::string& pakFile)
{
    return m_loadedExternalPaks.find(pakFile) != m_loadedExternalPaks.end();
}

void PakManager::UnloadAllPaks()
{
    PakState origState = m_state;
    m_state = PAK_STATE_UNLOAD_EXTERNAL;
    for (const auto& entry : m_loadedExternalPaks)
    {
        PakFunc6(entry.second, &doNothing);
        m_logger->info("Pak {} unloaded", entry.first);

        for (const auto& model : m_loadedExternalModels[entry.first])
        {
            m_logger->info("Unloading model {}", model->szName);
            CModelLoader_UnloadModel(m_modelLoader, model);
        }
    }

    m_loadedExternalModels.clear();
    m_loadMaterialArgs.clear();
    m_materialsToLoad.clear();
    m_texturesToLoad.clear();
    m_shadersToLoad.clear();
    m_loadedExternalPaks.clear();
    
    m_logger->info("Finished unloading models");
    m_state = origState;
}

void PakManager::WriteCacheToFile(const std::string& filename)
{
    ttf2sdk::PakCache pbCache;
    pbCache.set_cacheversion(PAK_CACHE_FILE_VERSION);
    pbCache.set_gamebuild(GetGameBuild());

    for (const auto& it : m_cachedMaterialData)
    {
        ttf2sdk::MaterialData* data = pbCache.add_materials();
        data->set_name(it.first);

        for (const auto& pakFile : it.second.pakFiles)
        {
            data->add_pakfiles(pakFile);
        }
        
        for (const auto& shaderName : it.second.shaderNames)
        {
            data->add_shadernames(shaderName);
        }

        for (const auto& texture : it.second.textures)
        {
            data->add_textures(texture);
        }
    }

    std::ofstream outFile(filename, std::ios::binary);
    if (!pbCache.SerializeToOstream(&outFile))
    {
        m_logger->error("Failed to write pak cache to disk");
    }
    else
    {
        m_logger->info("Wrote pak cache to {}", filename);
    }
}

void PakManager::WritePakCache()
{
    WriteCacheToFile((SDK().GetFSManager().GetBasePath() / "pakcache.dat").string());
}

void PakManager::MaterialFunc1Hook(CMaterialGlue* glue, MaterialData* data)
{
    SPDLOG_TRACE(m_logger, "MaterialFunc1: {}, glue = {}", (glue->name != NULL) ? glue->name : "NULL", (void*)glue);

    if (m_state == PAK_STATE_PRELOAD)
    {
        std::lock_guard<std::mutex> lock(m_materialsMutex);
        m_tempLoadedMaterials.emplace(glue);
        // NOTE: We take the glue pointer here because some if its fields will not get filled in
        // until later, and those are the ones we're really interested in.
    }

    m_matFunc1(glue, data);
}

void PakManager::TextureFunc1Hook(TextureInfo* info, int64_t a2, int64_t a3, int64_t a4)
{
    SPDLOG_TRACE(m_logger, "TextureFunc1: {}", (info->name != NULL) ? info->name : "NULL");

    if (m_state == PAK_STATE_PRELOAD)
    {
        std::lock_guard<std::mutex> lock(m_texturesMutex);
        m_tempLoadedTextures.emplace(info->name);
    }
    else if (m_state == PAK_STATE_SPAWN_EXTERNAL)
    {
        // Only load the texture if it's one we want to load
        if (m_texturesToLoad.find(info->name) != m_texturesToLoad.end())
        {
            SPDLOG_TRACE(m_logger, "Calling original TextureFunc1 for {}", info->name);
            m_texFunc1(info, a2, a3, a4);
        }
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(m_mapTexturesMutex);
            m_mapTextures.emplace(info->name);
        }

        m_texFunc1(info, a2, a3, a4);
    }
}

int64_t PakManager::TextureFunc2Hook(TextureInfo* info)
{
    SPDLOG_TRACE(m_logger, "TextureFunc2: {}", (info->name != NULL) ? info->name : "NULL");

    // If we're unloading externals, only unload ones we loaded
    if (m_state == PAK_STATE_SPAWN_EXTERNAL || m_state == PAK_STATE_UNLOAD_EXTERNAL)
    {
        if (m_texturesToLoad.find(info->name) != m_texturesToLoad.end())
        {
            if (info->texture2D != NULL && info->SRView != NULL)
            {
                SPDLOG_TRACE(m_logger, "Unloading texture: {}", info->name);
                m_texFunc2(info);
            }
        }
        return ERROR_SUCCESS;
    }
    else if (m_state != PAK_STATE_PRELOAD)
    {
        return m_texFunc2(info);
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

void PakManager::TextureFunc3Hook(TextureInfo* dst, TextureInfo* src, void* a3)
{
    const char* name = NULL;
    if (src != NULL)
    {
        name = src->name;
    }
    else if (dst != NULL)
    {
        name = dst->name;
    }

    SPDLOG_TRACE(m_logger, "TextureFunc3: {}", (name != NULL) ? name : "NULL");

    if (m_state == PAK_STATE_SPAWN_EXTERNAL || m_state == PAK_STATE_UNLOAD_EXTERNAL)
    {
        if (name != NULL)
        {
            if (m_texturesToLoad.find(name) != m_texturesToLoad.end())
            {
                SPDLOG_TRACE(m_logger, "Forwarding call to original TextureFunc3");
                m_texFunc3(dst, src, a3);
            }
        }
    }
    else if (m_state != PAK_STATE_PRELOAD)
    {
        m_texFunc3(dst, src, a3);
    }
}

void PakManager::ShaderFunc1Hook(ShaderInfo* info, int64_t a2)
{
    SPDLOG_TRACE(m_logger, "ShaderFunc1: {}, type = {}", (info->name != nullptr) ? info->name : "NULL", info->type);

    if (m_state == PAK_STATE_PRELOAD)
    {
        std::lock_guard<std::mutex> lock(m_shadersMutex);
        m_tempLoadedShaders.emplace(info->name);
    }

    if (((m_state == PAK_STATE_SPAWN_EXTERNAL || m_state == PAK_STATE_UNLOAD_EXTERNAL) && info->name != nullptr && m_shadersToLoad.find(info->name) != m_shadersToLoad.end()) || m_state == PAK_STATE_PRELOAD)
    {
        SPDLOG_TRACE(m_logger, "Blocking shader {} from loading", info->name);
        std::lock_guard<std::mutex> lock(m_dummyShaderMutex);
        m_loadingExtranousShader = true;
        m_shaderFunc1(info, a2);
        m_loadingExtranousShader = false;
    }
    else
    {
        m_shaderFunc1(info, a2);
    }
}

void PakManager::ShaderFunc2Hook(ShaderInfo* info)
{
    SPDLOG_TRACE(m_logger, "ShaderFunc2: {}, type = {}", (info->name != nullptr) ? info->name : "NULL", info->type);
    if ((m_state == PAK_STATE_SPAWN_EXTERNAL && info->name != nullptr && m_shadersToLoad.find(info->name) != m_shadersToLoad.end()) || m_state == PAK_STATE_PRELOAD)
    {
        SPDLOG_TRACE(m_logger, "Ignoring unload for shader {}", info->name);
        return;
    }

    m_shaderFunc2(info);
}

int32_t PakManager::PakFunc3Hook(const char* src, PakAllocFuncs* allocFuncs, int unk)
{
    int32_t retVal = PakFunc3(src, allocFuncs, unk);
    SPDLOG_TRACE(m_logger, "PakFunc3: src = {}, ret = {}", src, retVal);
    return retVal;
}

int64_t PakManager::PakFunc6Hook(int32_t pakRef, void* a2)
{
    // PakFunc6 is the unload function, and m_savedPakRef2 is the reference to the main
    // pak for the current level. If we start unloading that, we want to unload our
    // custom loaded paks too.
    if (m_state == PAK_STATE_IN_LOAD_MAP_PAK && pakRef == m_savedPakRef2)
    {
        m_logger->info("Engine unloading current map pak, unloading external paks");
        m_mapTextures.clear();
        UnloadAllPaks();
        for (model_t* model : m_levelModels)
        {
            m_logger->info("Unloading level model {}", model->szName);
            CModelLoader_UnloadModel(m_modelLoader, model);
        }
        m_levelModels.clear();
    }

    int64_t retVal = PakFunc6(pakRef, a2);
    SPDLOG_TRACE(m_logger, "PakFunc6: pakRef = {}, a2 = {}, ret = {}", pakRef, a2, retVal);
    return retVal;
}

int64_t PakManager::PakFunc9Hook(int32_t pakRef, void* a2, void* cb)
{
    int64_t retVal = PakFunc9(pakRef, a2, cb);
    SPDLOG_TRACE(m_logger, "PakFunc9: pakRef = {}, a2 = {}, cb = {}, ret = {}", pakRef, a2, cb, retVal);
    return retVal;
}

int64_t PakManager::PakFunc13Hook(const char* name)
{
    int64_t retVal = PakFunc13(name);
    SPDLOG_TRACE(m_logger, "PakFunc13: name = {}, retval = {}", name, (void*)retVal);
    return retVal;
}

int64_t PakManager::PrecacheModelHook(IVEngineServer* engineServer, const char* modelName)
{
    SPDLOG_TRACE(m_logger, "IVEngineServer::PrecacheModel: {}", modelName);
    int64_t retVal = IVEngineServer_PrecacheModel(engineServer, modelName);
    SPDLOG_TRACE(m_logger, "IVEngineServer::PrecacheModel returned {}", retVal);
    return retVal;
}

int64_t PakManager::Studio_LoadModelHook(void* modelLoader, model_t* model)
{
    // Capture the global model loader for unloading models later
    if (m_modelLoader == nullptr)
    {
        m_modelLoader = modelLoader;
    }

    m_savedModelPtr = model;
    SPDLOG_TRACE(m_logger, "Studio_LoadModel: model = {}, name = {}", (void*)model, model->szName);
    int64_t retVal = Studio_LoadModel(modelLoader, model);
    m_savedModelPtr = nullptr;

    if (m_state == PAK_STATE_NONE)
    {
        m_levelModels.insert(model);
    }

    return retVal;
}

std::string GetMaterialPath(int32_t* phdr, int32_t index)
{
    int32_t* v14 = (int32_t*)((char*)&phdr[11 * index] + phdr[53]);
    char *v15 = (char *)v14 + *v14;
    if (*v15 == '\\' || *v15 == '/')
    {
        ++v15;
    }
    char path[MAX_PATH];
    strcpy_s(path, v15);
    Util::FixSlashes(path, '\\');
    return std::string(path);
}

uint64_t PakManager::LoadMaterialsHook(int64_t a1, int32_t* phdr, int64_t a3, studioloddata_t* lodData, uint32_t a5)
{
    m_studioRenderContext = (void*)a1;
    if (m_state == PAK_STATE_SPAWN_EXTERNAL)
    {
        int32_t count = phdr[52];
        SPDLOG_TRACE(m_logger, "LoadMaterials: count = {}", count);
        if (m_loadMaterialArgs.find(m_savedModelPtr) != m_loadMaterialArgs.end())
        {
            m_logger->error("Load material args already set for {}", (void*)m_savedModelPtr);
        }

        LoadMaterialArgs& args = m_loadMaterialArgs[m_savedModelPtr];
        args.phdr = phdr;
        args.a3 = a3;
        args.lodData = lodData;
        args.a5 = a5;

        bool needsPakReload = false;
        std::vector<std::string> pakCandidates;
        std::unordered_set<std::string> newMaterialsToLoad;
        std::unordered_set<std::string> newTexturesToLoad;
        std::unordered_set<std::string> newShadersToLoad;
        for (int32_t i = 0; i < count; i++)
        {
            // Get the material path
            std::string path = GetMaterialPath(phdr, i);
            SPDLOG_TRACE(m_logger, "\ti = {}, path = {}", i, path);

            // Ensure that we have cached data for this material
            if (m_cachedMaterialData.find(path) == m_cachedMaterialData.end())
            {
                m_logger->error("Failed to find cached material data for: {}", path);
                continue;
            }

            // Retrieve the cached data
            const auto& matData = m_cachedMaterialData[path];

            // If we've loaded this material before, no need for more processing
            if (m_materialsToLoad.find(path) != m_materialsToLoad.end() ||
                newMaterialsToLoad.find(path) != newMaterialsToLoad.end())
            {
                continue;
            }

            // Add material to list to load
            newMaterialsToLoad.insert(path);

            // Add textures
            for (const auto& tex : matData.textures)
            {
                if (m_mapTextures.find(tex) == m_mapTextures.end())
                {
                    newTexturesToLoad.insert(tex);
                }
                else
                {
                    m_logger->debug("Skipping texture {}, already loaded on map", tex);
                }
            }

            // Add shaders
            for (const auto& shader : matData.shaderNames)
            {
                newShadersToLoad.insert(shader);
            }

            // Calculate the intersection of the current candidates and the paks for this material
            if (pakCandidates.size() == 0)
            {
                pakCandidates = matData.pakFiles;
            }
            else
            {
                std::sort(pakCandidates.begin(), pakCandidates.end());
                auto end = std::set_intersection(
                    pakCandidates.begin(), pakCandidates.end(),
                    matData.pakFiles.begin(), matData.pakFiles.end(),
                    pakCandidates.begin() // intersection is written into pakCandidates
                );
                pakCandidates.erase(end, pakCandidates.end()); // erase redundant elements
            }

            needsPakReload = true;
        }

        if (needsPakReload)
        {
            if (pakCandidates.size() == 0)
            {
                m_logger->error("Failed to find a suitable pak");
            }
            else
            {
                std::string expectedPak = SDK().GetFSManager().GetLastMapReadFrom() + ".rpak";

                // If there are multiple options, see if we can use the current level
                std::string pakName;
                if (std::find(pakCandidates.begin(), pakCandidates.end(), m_currentLevelPak) != pakCandidates.end())
                {
                    m_logger->info("Current level found in pak candidates, using that");
                    pakName = m_currentLevelPak;
                }
                else if (std::find(pakCandidates.begin(), pakCandidates.end(), expectedPak) == pakCandidates.end())
                {
                    m_logger->warn("Expected pak ({}) not found in candidates ({})", expectedPak, Util::ConcatStrings(pakCandidates, ","));
                    pakName = pakCandidates[0];
                }
                else
                {
                    pakName = expectedPak;
                }

                if (pakName == m_currentLevelPak)
                {
                    m_logger->info("Candidate pak was current level, not loading anything");
                    // TODO: Should we still log an external model here?
                }
                else
                {
                    ReloadExternalPak(pakName, newMaterialsToLoad, newTexturesToLoad, newShadersToLoad);

                    // Capture all the models that we spawn in extenal mode to free them later
                    m_loadedExternalModels[pakName].insert(m_savedModelPtr);
                    // TODO: Yeah this is a problem, if mat already loaded still need to somehow mark this
                    // as an external model that needs to get unloaded
                }
            }
        }
    }

    return CStudioRenderContext_LoadMaterials(a1, phdr, a3, lodData, a5);
}

int64_t PakManager::LoadMaterials_SubFunc_Hook(int* pMaterialFlags, int32_t* phdr, int64_t a3, unsigned int a4)
{
    // The sub func inside load materials updates the material flags by reading some data from pVtxHeader.
    // Since pVtxHeader might be freed by the time this gets called again, we just need to
    // not call the function and restore the material flags manually.
    if (m_state == PAK_STATE_RESETTING_MATERIALS)
    {
        memcpy(pMaterialFlags, m_savedMaterialFlags[phdr].data(), m_savedMaterialFlags[phdr].size() * sizeof(int));
        return 0;
    }
    else
    {
        return LoadMaterials_SubFunc(pMaterialFlags, phdr, a3, a4);
    }
}

int64_t PakManager::SetModelHook(void* ent, const char* modelName)
{
    SPDLOG_TRACE(m_logger, "SetModel: ent = {}, model = {}", ent, modelName);
    return CBaseEntity_SetModel(ent, modelName);
}

bool PakManager::LoadMapPakHook(const char* name)
{
    SPDLOG_TRACE(m_logger, "LoadMapPak: {}", name);
    
    // Get the current level from name
    std::string strName(name);
    size_t found = strName.find_last_of(".bsp");
    if (found != std::string::npos)
    {
        strName = strName.substr(0, found);
    }

    m_currentLevelPak = strName + ".rpak";

    m_savedPakRef2 = m_pakRefs[2];
    m_state = PAK_STATE_IN_LOAD_MAP_PAK;
    bool ret = LoadMapPak(name);
    m_state = PAK_STATE_NONE;
    m_savedPakRef2 = -1;

    return ret;
}

void PakManager::CreateDummyShaders(ID3D11Device** ppD3DDevice)
{
    const char* shaderText = "void main() { return; }";
    const char* geometryShaderText = "struct GS_INPUT {}; [maxvertexcount(4)] void main(point GS_INPUT a[1]) { return; }";
    const char* computeShaderText = "[numthreads(1, 1, 1)] void main() { return; }";

    m_dummyGeometryShader = nullptr;
    m_dummyPixelShader = nullptr;
    m_dummyComputeShader = nullptr;
    m_dummyVertexShader = nullptr;

    ID3DBlob* vertexShaderBlob = nullptr;
    HRESULT result = D3DCompile(
        shaderText,
        strlen(shaderText),
        "TTF2SDK_VS",
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &vertexShaderBlob,
        nullptr
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to compile vertex shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Vertex shader blob: {}", (void*)vertexShaderBlob);
    }

    ID3DBlob* pixelShaderBlob = nullptr;
    result = D3DCompile(
        shaderText,
        strlen(shaderText),
        "TTF2SDK_PS",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &pixelShaderBlob,
        nullptr
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to compile pixel shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Pixel shader blob: {}", (void*)pixelShaderBlob);
    }

    ID3DBlob* geometryShaderBlob = nullptr;
    result = D3DCompile(
        geometryShaderText,
        strlen(geometryShaderText),
        "TTF2SDK_GS",
        nullptr,
        nullptr,
        "main",
        "gs_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &geometryShaderBlob,
        nullptr
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to compile geometry shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Geometry shader blob: {}", (void*)geometryShaderBlob);
    }

    ID3DBlob* computeShaderBlob = nullptr;
    result = D3DCompile(
        computeShaderText,
        strlen(computeShaderText),
        "TTF2SDK_CS",
        nullptr,
        nullptr,
        "main",
        "cs_5_0",
        D3DCOMPILE_OPTIMIZATION_LEVEL0,
        0,
        &computeShaderBlob,
        nullptr
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to compile compute shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Compute shader blob: {}", (void*)computeShaderBlob);
    }

    ID3D11Device* dev = *ppD3DDevice;
    result = dev->lpVtbl->CreateGeometryShader(
        dev,
        geometryShaderBlob->lpVtbl->GetBufferPointer(geometryShaderBlob),
        geometryShaderBlob->lpVtbl->GetBufferSize(geometryShaderBlob),
        nullptr,
        &m_dummyGeometryShader
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to create geometry shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Geometry shader: {}", (void*)m_dummyGeometryShader);
    }

    result = dev->lpVtbl->CreateVertexShader(
        dev,
        vertexShaderBlob->lpVtbl->GetBufferPointer(vertexShaderBlob),
        vertexShaderBlob->lpVtbl->GetBufferSize(vertexShaderBlob),
        nullptr,
        &m_dummyVertexShader
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to create vertex shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Vertex shader: {}", (void*)m_dummyVertexShader);
    }

    result = dev->lpVtbl->CreatePixelShader(
        dev,
        pixelShaderBlob->lpVtbl->GetBufferPointer(pixelShaderBlob),
        pixelShaderBlob->lpVtbl->GetBufferSize(pixelShaderBlob),
        NULL,
        &m_dummyPixelShader
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to create pixel shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Pixel shader: {}", (void*)m_dummyPixelShader);
    }

    result = dev->lpVtbl->CreateComputeShader(
        dev,
        computeShaderBlob->lpVtbl->GetBufferPointer(computeShaderBlob),
        computeShaderBlob->lpVtbl->GetBufferSize(computeShaderBlob),
        NULL,
        &m_dummyComputeShader
    );

    if (!SUCCEEDED(result))
    {
        m_logger->error("Failed to create compute shader");
        return;
    }
    else
    {
        SPDLOG_DEBUG(m_logger, "Compute shader: {}", (void*)m_dummyComputeShader);
    }
}

HRESULT STDMETHODCALLTYPE PakManager::CreateVertexShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader)
{
    if (m_loadingExtranousShader && m_dummyVertexShader != nullptr)
    {
        *ppVertexShader = m_dummyVertexShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreateVertexShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
    }
}

HRESULT STDMETHODCALLTYPE PakManager::CreateGeometryShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
{
    if (m_loadingExtranousShader && m_dummyGeometryShader != nullptr)
    {
        *ppGeometryShader = m_dummyGeometryShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreateGeometryShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppGeometryShader);
    }
}

HRESULT STDMETHODCALLTYPE PakManager::CreatePixelShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader)
{
    if (m_loadingExtranousShader && m_dummyPixelShader != nullptr)
    {
        *ppPixelShader = m_dummyPixelShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreatePixelShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);
    }
}

HRESULT STDMETHODCALLTYPE PakManager::CreateComputeShader_Hook(ID3D11Device* This, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader)
{
    if (m_loadingExtranousShader && m_dummyComputeShader != nullptr)
    {
        *ppComputeShader = m_dummyComputeShader;
        return ERROR_SUCCESS;
    }
    else
    {
        return ID3D11Device_CreateComputeShader(This, pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader);
    }
}
