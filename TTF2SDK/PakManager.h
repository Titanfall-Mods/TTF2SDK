#pragma once

struct TypeRegistration
{
    char type[4];
    unsigned int unk;
    const char* niceName;
    void* funcs[3];
    unsigned char unk2[56];

    bool IsValid()
    {
        return type[0] != 0;
    }

    bool IsTypeEqual(const char* testType)
    {
        return strncmp(type, testType, _countof(type)) == 0;
    }
};

struct ShaderInfo
{
    const char* name;
    int32_t type;
};

#pragma pack(push,1)
struct TextureInfo
{
    char unknown[8];
    const char* name;
    char unknown2[18];
    bool someBool;
    char unknown3[245];
    ID3D11Texture2D* texture2D;
    ID3D11ShaderResourceView* SRView;
};
#pragma pack(pop)

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
    void* data;
    int size;
};

enum PakState
{
    PAK_STATE_NONE,
    PAK_STATE_PRELOAD,
    PAK_STATE_SPAWN_EXTERNAL,
    PAK_STATE_IN_LOAD_MAP_PAK,
    PAK_STATE_UNLOAD_EXTERNAL,
};

struct CachedMaterialData
{
    std::vector<std::string> pakFiles;
    std::vector<std::string> shaderNames;
    std::vector<std::string> textures;
};

class PakManager;
template <typename T> class HookedRegistrationFunc;
template<typename R, typename ...Args>
class HookedRegistrationFunc<R(PakManager::*)(Args...)>
{
    bool m_hooked = false;
    TypeRegistration* m_registration = nullptr;
    int m_funcNum = 0;
    R(*m_origFunc)(Args...) = nullptr;

public:
    void Hook(TypeRegistration* registration, int funcNum, R(*hookFunc)(Args...))
    {
        if (m_hooked)
        {
            return;
        }

        m_funcNum = funcNum;
        m_registration = registration;
        m_origFunc = (R(*)(Args...))registration->funcs[funcNum];
        registration->funcs[funcNum] = (void*)hookFunc;
        m_hooked = true;
    }

    ~HookedRegistrationFunc()
    {
        if (m_hooked)
        {
            m_registration->funcs[m_funcNum] = m_origFunc;
            m_hooked = false;
        }
    }

    R operator()(Args... args)
    {
        return m_origFunc(args...);
    }
};

struct PakAllocFuncs
{
    void* (*Alloc)(int64_t a1, size_t size, size_t alignment);
    void(*Free)(int64_t a1, void* ptr);
};

struct model_t
{
    unsigned char fnHandle[4];
    char szName[250];
};

struct CPrecacheItem
{
    int64_t unk;
    model_t* model;
};

struct CGameServer
{
    char unknown[6800840];
    CPrecacheItem model_precache[1024];
};

struct CClientState
{
    char unknown[79400];
    CPrecacheItem model_precache[1024];
};

typedef CClientState* (*tClientStateFunc)();

class PakManager
{
public:
    PakManager(ConCommandManager& conCommandManager, SourceInterface<IVEngineServer> engineServer);
    void TEMP_SetModelsOnEnts(const CCommand& args);
    void PrintRegistrations(const CCommand& args);
    void PrintPakRefs(const CCommand& args);
    void PrintCachedMaterialData(const CCommand& args);
    void PrintExternalModels(const CCommand& args);
    void PrintCurrentLevelPak(const CCommand& args);

    void SetExternalSpawnMode(const CCommand& args);

    void AddTextureIfExists(CachedMaterialData& data, const std::string& matName, const char* ext);
    void ResolveMaterials(const std::string& pakName);
    void PreloadPak(const char* name);
    void PreloadAllPaks();
    
    void ReloadExternalPak(const std::string& pakFile, std::unordered_set<std::string>& newMaterialsToLoad, std::unordered_set<std::string>& newTexturesToLoad, std::unordered_set<std::string>& newShadersToLoad);
    void LoadExternalPak(const std::string& pakFile);
    bool IsExternalPakLoaded(const std::string& pakFile);
    void UnloadAllPaks();

    void WriteCacheToFile(const std::string& filename);

    void MaterialFunc1Hook(CMaterialGlue* glue, MaterialData* data);
    void TextureFunc1Hook(TextureInfo* info, int64_t a2, int64_t a3, int64_t a4);
    int64_t TextureFunc2Hook(TextureInfo* info);
    void TextureFunc3Hook(TextureInfo* dst, TextureInfo* src, void* a3);
    void ShaderFunc1Hook(ShaderInfo* info, int64_t a2);

    int32_t PakFunc3Hook(const char* src, PakAllocFuncs* allocFuncs, int unk);
    int64_t PakFunc6Hook(int32_t pakRef, void* a2);
    int64_t PakFunc9Hook(int32_t pakRef, void* a2, void* cb);
    int64_t PakFunc13Hook(const char* name);

    int64_t PrecacheModelHook(IVEngineServer* engineServer, const char* modelName);
    int64_t Studio_LoadModelHook(void* modelLoader, model_t* model);
    uint64_t LoadMaterialsHook(int64_t a1, int32_t* phdr, int64_t a3, int64_t a4, uint32_t a5);
    int64_t SetModelHook(void* ent, const char* modelName);

    bool LoadMapPakHook(const char* name);

private:
    std::shared_ptr<spdlog::logger> m_logger;

    SourceInterface<IVModelInfo> m_modelInfo;
    CGameServer* m_gameServer;
    CClientState* m_clientState;
    TypeRegistration* m_typeRegistrations;
    
    int32_t* m_pakRefs;
    PakState m_state;
    std::string m_currentLevelPak;

    model_t* m_savedModelPtr;
    int32_t m_savedPakRef2;

    std::mutex m_shadersMutex;
    std::unordered_set<std::string> m_tempLoadedShaders;

    std::mutex m_texturesMutex;
    std::unordered_set<std::string> m_tempLoadedTextures;

    std::mutex m_materialsMutex;
    std::unordered_set<CMaterialGlue*> m_tempLoadedMaterials;

    std::unordered_map<std::string, CachedMaterialData> m_cachedMaterialData;

    void* m_modelLoader = nullptr;
    std::unordered_map<std::string, std::unordered_set<model_t*>> m_loadedExternalModels; // pak name => { model ptrs }
    std::unordered_set<std::string> m_materialsToLoad;
    std::unordered_set<std::string> m_texturesToLoad;
    std::unordered_set<std::string> m_shadersToLoad;
    std::unordered_map<std::string, int32_t> m_loadedExternalPaks;
    std::unordered_map<std::string, std::unordered_set<void*>> m_externalModelToEntityMap; // model name => { entity }

    HookedRegistrationFunc<decltype(&MaterialFunc1Hook)> m_matFunc1;
    HookedRegistrationFunc<decltype(&TextureFunc1Hook)> m_texFunc1;
    HookedRegistrationFunc<decltype(&TextureFunc2Hook)> m_texFunc2;
    HookedRegistrationFunc<decltype(&TextureFunc3Hook)> m_texFunc3;
    HookedRegistrationFunc<decltype(&ShaderFunc1Hook)> m_shaderFunc1;
};
