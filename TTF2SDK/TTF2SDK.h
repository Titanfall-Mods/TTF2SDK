#pragma once

typedef long SQInteger;

// TODO: Combine these
struct ClientVM
{
    unsigned char Data[8];
    HSQUIRRELVM sqvm;
};

// TODO: Combine these
struct ServerVM
{
    unsigned char Data[8];
    HSQUIRRELVM sqvm;
};

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

class FileReplacementManager
{
    std::string m_searchPath;
    //std::shared_mutex m_filesMutex;
    //std::unordered_set<std::string> m_filesToOverride;

public:
    // TODO: Must include trailing slash
    FileReplacementManager(const std::string& searchPath) : m_searchPath(searchPath)
    {
        // TODO: Move this into a function that C# can call. It should call tolower on the input and also add both slash versions.
        // Alternatively, just replace the set comparison function with a case insensitive function and one that doesn't care about slashes.
        //m_filesToOverride.emplace("scripts\\damage\\damageflags.txt");
        //m_filesToOverride.emplace("scripts/damage/damageflags.txt");
        //m_filesToOverride.emplace("scripts/vscripts/sp/_sp_sh_init.gnut");
        //m_filesToOverride.emplace("scripts\\vscripts\\sp\\_sp_sh_init.gnut");
    }

    bool ShouldReplaceFile(const std::string& path)
    {
        //std::shared_lock<std::shared_mutex> lock(m_filesMutex);
        //if (m_filesToOverride.find(path) != m_filesToOverride.end())
        //{
        //    m_logger->info("IFileSystem::ReadFromCache: blocking cache response for {}", path);
        //    return false;
        //}
        std::ifstream f(m_searchPath + path);
        return f.good();
    }

    const std::string& GetSearchPath()
    {
        return m_searchPath;
    }
};

struct VPKInfo
{
    unsigned char unknown[5];
    char path[255];
};

class TTF2SDK
{
private:
    std::shared_ptr<spdlog::logger> m_logger;

    ClientVM** m_ppClientVM = nullptr;
    ServerVM** m_ppServerVM = nullptr;
    
    
    FileReplacementManager m_replacementManager;

    std::mutex m_mutex;
    std::atomic_bool m_shouldRunClientCode = false;
    std::string m_clientCodeToRun = "";
    std::atomic_bool m_shouldRunServerCode = false;
    std::string m_serverCodeToRun = "";

    SourceInterface<IFileSystem> m_fileSystem;
    SourceInterface<IEngineClient> m_engineClient;
    SourceInterface<IEngineServer> m_engineServer;

public:
    ID3D11Device** m_ppD3D11Device = nullptr;

    TTF2SDK();
    ~TTF2SDK();

    void RunFrameHook(double absTime, float frameTime);

    // Filesystem stuff
    void AddSearchPathHook(IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType);
    bool ReadFromCacheHook(IFileSystem* fileSystem, const char* path, void* result);
    __int32* ReadFileFromVPKHook(VPKInfo* vpkInfo, __int32* b, char* filename);
    unsigned int* MountVPKHook(IFileSystem* fileSystem, const char* vpkPath);

    template<ExecutionContext context>
    SQInteger BasePrintHook(HSQUIRRELVM v);
    void PrintFunc(HSQUIRRELVM v, const SQChar* source, const SQChar* s, va_list args);

    template<ExecutionContext context>
    void CompilerErrorHook(HSQUIRRELVM v, const SQChar* sErr, const SQChar* sSource, SQInteger line, SQInteger column);

    void compileShaders();

    HSQUIRRELVM GetClientSQVM()
    {
        if (*m_ppClientVM != nullptr)
        {
            return (*m_ppClientVM)->sqvm;
        }

        return nullptr;
    }

    HSQUIRRELVM GetServerSQVM()
    {
        if (*m_ppServerVM != nullptr)
        {
            return (*m_ppServerVM)->sqvm;
        }

        return nullptr;
    }

    void SetClientCode(const std::string& code)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_clientCodeToRun = code;
        m_shouldRunClientCode = true;
    }

    std::string GetClientCode()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_shouldRunClientCode = false;
        return m_clientCodeToRun;
    }

    void SetServerCode(const std::string& code)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_serverCodeToRun = code;
        m_shouldRunServerCode = true;
    }

    std::string GetServerCode()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_shouldRunServerCode = false;
        return m_serverCodeToRun;
    }
};

TTF2SDK& SDK();
bool SetupSDK();
