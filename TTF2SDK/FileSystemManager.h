#pragma once

struct VPKInfo
{
    unsigned char unknown[5];
    char path[255];
};

class FileSystemManager
{
private:
    static std::regex s_mapFromVPKRegex;

    std::shared_ptr<spdlog::logger> m_logger;
    SourceInterface<IFileSystem> m_engineFileSystem;
    std::string m_searchPath;
    std::vector<std::string> m_mapVPKs;
    std::vector<std::string> m_mapNames;

    void CacheMapVPKs();
    bool ShouldReplaceFile(const std::string& path);

public:
    FileSystemManager(const std::string& searchPath);

    void AddSearchPathHook(IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType);
    bool ReadFromCacheHook(IFileSystem* fileSystem, const char* path, void* result);
    __int32* ReadFileFromVPKHook(VPKInfo* vpkInfo, __int32* b, char* filename);
    unsigned int* MountVPKHook(IFileSystem* fileSystem, const char* vpkPath);
    const std::vector<std::string>& GetMapNames();
};
