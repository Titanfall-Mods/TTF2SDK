#include "stdafx.h"

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
SigScanFunc<void> PakFunc1("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x8B\x01", "xxxx?xxxx?xxxx?xxxxxxxx?xx");
HookedFunc<int32_t, const char*, PakAllocFuncs*, int> PakFunc3("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF1\x48\x8D\x0D\x00\x00\x00\x00", "xxxx?xxxx?xxxx?xxxxxx????");
HookedFunc<int64_t, int32_t, void*> PakFunc6("rtech_game.dll", "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xDA\x8B\xF9", "xxxx?xxxx?xxxxx");
HookedFunc<int64_t, int32_t, void*, void*> PakFunc9("rtech_game.dll", "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x8B\xD9", "xxxx?xxxx?xxxx?xxxx?xx");
HookedFunc<int64_t, const char*> PakFunc13("rtech_game.dll", "\x48\x83\xEC\x00\x48\x8D\x15\x00\x00\x00\x00", "xxx?xxx????");

const int MAT_REG_INDEX = 4;
const int TEX_REG_INDEX = 12;
const int SHADER_REG_INDEX = 13;

void TEMP_preload(const CCommand& args)
{
    PakMan().PreloadAllPaks();
}

PakManager::PakManager(ConCommandManager& conCommandManager)
{
    m_logger = spdlog::get("logger");
    m_state = PAK_STATE_NONE;

    int64_t base = (int64_t)PakFunc1.GetFuncPtr() + 33;
    int32_t offset = *((int32_t*)(base - 4));
    m_typeRegistrations = (TypeRegistration*)(base + offset);
    m_logger->debug("Type Registrations: {}", (void*)m_typeRegistrations);

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
    m_logger->debug("Pak refs: {}", (void*)m_pakRefs);

    m_matFunc1.Hook(&m_typeRegistrations[MAT_REG_INDEX], 0, WRAPPED_MEMBER(MaterialFunc1Hook));
    m_texFunc1.Hook(&m_typeRegistrations[TEX_REG_INDEX], 0, WRAPPED_MEMBER(TextureFunc1Hook));
    m_shaderFunc1.Hook(&m_typeRegistrations[SHADER_REG_INDEX], 0, WRAPPED_MEMBER(ShaderFunc1Hook));

    PakFunc6.Hook(WRAPPED_MEMBER(PakFunc6Hook));
#ifdef _DEBUG
    PakFunc3.Hook(WRAPPED_MEMBER(PakFunc3Hook));
#endif

    conCommandManager.RegisterCommand("print_registrations", WRAPPED_MEMBER(PrintRegistrations), "Print type registrations for pak system", 0);
    conCommandManager.RegisterCommand("print_pak_refs", WRAPPED_MEMBER(PrintPakRefs), "Print engine pak references", 0);
    conCommandManager.RegisterCommand("print_cached_materials", WRAPPED_MEMBER(PrintCachedMaterialData), "Print cached material data", 0);

    conCommandManager.RegisterCommand("TEMP_preload", TEMP_preload, "TEMP preload", 0);
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
    // Clear previous preload data
    m_tempLoadedShaders.clear();
    m_tempLoadedTextures.clear();
    m_tempLoadedMaterials.clear();

    m_logger->debug("Preloading pak: {}", name);

    // Initialise
    int32_t pakRef = PakFunc3(name, &g_SDKAllocFuncs, 3);
    if (pakRef == -1)
    {
        m_logger->error("Failed to preload pak {}, PakFunc3 failed", name);
        return;
    }

    // Set state to preload
    m_state = PAK_STATE_PRELOAD;

    // Load
    int64_t result = PakFunc9(pakRef, &doNothing, &doNothing);
    if (result != 1)
    {
        m_logger->error("Failed to preload pak {}, PakFunc9 failed ({})", name, result);
        return;
    }

    // Restore state
    m_state = PAK_STATE_NONE;

    // Build data structure with all loaded assets
    ResolveMaterials(name);

    // Free pak
    PakFunc6(pakRef, &doNothing);

    m_logger->info("Finished preloading pak: {}", name);
}

void PakManager::PreloadAllPaks()
{
    // TODO: This function causes a large amount of memory usage.
    // Probably need to set it up so that it will load one, run a few frames, then go to the next one
    // to allow for the memory to get freed before moving on.
    m_cachedMaterialData.clear();

    const auto& mapNames = SDK().GetFSManager().GetMapNames();
    for (const auto& map : mapNames)
    {
        std::string pakName = map + ".rpak";
        PreloadPak(pakName.c_str());
    }
}

void PakManager::MaterialFunc1Hook(CMaterialGlue* glue, MaterialData* data)
{
    m_logger->trace("MaterialFunc1: {}, glue = {}", (glue->name != NULL) ? glue->name : "NULL", (void*)glue);

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
    m_logger->trace("TextureFunc1: {}", (info->name != NULL) ? info->name : "NULL");

    if (m_state == PAK_STATE_PRELOAD)
    {
        std::lock_guard<std::mutex> lock(m_texturesMutex);
        m_tempLoadedTextures.emplace(info->name);
    }

    m_texFunc1(info, a2, a3, a4);
}

void PakManager::ShaderFunc1Hook(ShaderInfo* info, int64_t a2)
{
    m_logger->trace("ShaderFunc1: {}, type = {}", (info->name != NULL) ? info->name : "NULL", info->type);

    if (m_state == PAK_STATE_PRELOAD)
    {
        std::lock_guard<std::mutex> lock(m_shadersMutex);
        m_tempLoadedShaders.emplace(info->name);
    }

    m_shaderFunc1(info, a2);
}

int32_t PakManager::PakFunc3Hook(const char* src, PakAllocFuncs* allocFuncs, int unk)
{
    int32_t retVal = PakFunc3(src, allocFuncs, unk);
    m_logger->trace("PakFunc3: src = {}, ret = {}", src, retVal);
    return retVal;
}

int64_t PakManager::PakFunc6Hook(int32_t pakRef, void* a2)
{
    // PakFunc6 is the unload function, and m_pakRefs[2] is the reference to the main
    // pak for the current level. If we start unloading that, we want to unload our
    // custom loaded paks too.
    if (pakRef == m_pakRefs[2])
    {
        // TODO: unloadPaks();
    }

    int64_t retVal = PakFunc6(pakRef, a2);
    m_logger->trace("PakFunc6: pakRef = {}, a2 = {}, ret = {}", pakRef, a2, retVal);
    return retVal;
}

int64_t PakManager::PakFunc9Hook(int32_t pakRef, void* a2, void* cb)
{
    int64_t retVal = PakFunc9(pakRef, a2, cb);
    m_logger->trace("PakFunc9: pakRef = {}, a2 = {}, cb = {}, ret = {}", pakRef, a2, cb, retVal);
    return retVal;
}

int64_t PakManager::PakFunc13Hook(const char* name)
{
    int64_t retVal = PakFunc13(name);
    m_logger->trace("PakFunc13: name = {}, retval = {}", name, retVal);
    return retVal;
}
