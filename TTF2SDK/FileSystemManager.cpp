#include "stdafx.h"

namespace fs = std::experimental::filesystem;

FileSystemManager& FSManager()
{
    return SDK().GetFSManager();
}

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&FileSystemManager::##name), &FileSystemManager::##name, decltype(&FSManager), &FSManager>::Call

HookedVTableFunc<decltype(&IFileSystem::VTable::AddSearchPath), &IFileSystem::VTable::AddSearchPath> IFileSystem_AddSearchPath;
HookedVTableFunc<decltype(&IFileSystem::VTable::ReadFromCache), &IFileSystem::VTable::ReadFromCache> IFileSystem_ReadFromCache;
HookedVTableFunc<decltype(&IFileSystem::VTable::MountVPK), &IFileSystem::VTable::MountVPK> IFileSystem_MountVPK;
HookedFunc<FileHandle_t, VPKData*, __int32*, const char*> ReadFileFromVPK("filesystem_stdio.dll", "\x48\x89\x5C\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x49\x8B\xC0\x48\x8B\xDA", "xxxx?xxxx????xxxxxx");

std::regex FileSystemManager::s_mapFromVPKRegex("client_(.+)\\.bsp");

void FileSystemManager::TEMP_DumpAll(const CCommand& args)
{
    for (const auto& vpk : m_mapVPKs)
    {
        FSManager().DumpVPKScripts(vpk);
    }
}

FileSystemManager::FileSystemManager(const std::string& basePath, ConCommandManager& conCommandManager) :
    m_engineFileSystem("filesystem_stdio.dll", "VFileSystem017"),
    m_basePath(basePath),
    m_compiledPath(basePath + "compiled_assets\\"),
    m_dumpPath(basePath + "assets_dump\\"),
    m_modsPath(basePath + "mods\\"),
    m_devPath(basePath + "development\\")
{
    m_logger = spdlog::get("logger");

    CacheMapVPKs();

    // Hook functions
    IFileSystem_AddSearchPath.Hook(m_engineFileSystem->m_vtable, WRAPPED_MEMBER(AddSearchPathHook));
    IFileSystem_ReadFromCache.Hook(m_engineFileSystem->m_vtable, WRAPPED_MEMBER(ReadFromCacheHook));
    IFileSystem_MountVPK.Hook(m_engineFileSystem->m_vtable, WRAPPED_MEMBER(MountVPKHook));
    ReadFileFromVPK.Hook(WRAPPED_MEMBER(ReadFileFromVPKHook));
    conCommandManager.RegisterCommand("TEMP_DumpAll", WRAPPED_MEMBER(TEMP_DumpAll), "dump everything", 0);
}

void FileSystemManager::CacheMapVPKs()
{
    for (auto& file : fs::directory_iterator("vpk"))
    {
        if (!fs::is_regular_file(file))
        {
            continue;
        }

        std::string fileName = file.path().filename().generic_string();
        std::smatch m;
        std::regex_search(fileName, m, s_mapFromVPKRegex);
        if (!m.empty())
        {
            std::string path = "vpk/" + m[0].str();
            if (std::find(m_mapVPKs.begin(), m_mapVPKs.end(), path) == m_mapVPKs.end())
            {
                m_logger->info("Found map VPK: {}", path);
                m_mapVPKs.emplace_back(path);
                m_mapNames.emplace_back(m[1].str());
            }
        }
    }
}

// TODO: Do we maybe need to add the search path in a frame hook or will this do?
// TODO: If the search path has been added and this class is destroyed, should remove the search path
void FileSystemManager::AddSearchPathHook(IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType)
{
    SPDLOG_TRACE(m_logger, "IFileSystem::AddSearchPath: path = {}, pathID = {}, addType = {}", pPath, pathID != nullptr ? pathID : "", addType);

    // Add the path as intended
    IFileSystem_AddSearchPath(fileSystem, pPath, pathID, addType);

    // Add our search path to the head again to make sure we're first
    IFileSystem_AddSearchPath(fileSystem, m_compiledPath.c_str(), "GAME", PATH_ADD_TO_HEAD);
}

bool FileSystemManager::ReadFromCacheHook(IFileSystem* fileSystem, const char* path, void* result)
{
    // If the path is one of our replacements, we will not allow the cache to respond
    if (ShouldReplaceFile(path))
    {
        SPDLOG_DEBUG(m_logger, "IFileSystem::ReadFromCache: blocking cache response for {}", path);
        return false;
    }

    bool res = IFileSystem_ReadFromCache(fileSystem, path, result);
    SPDLOG_TRACE(m_logger, "IFileSystem::ReadFromCache: path = {}, res = {}", path, res);

    return res;
}

FileHandle_t FileSystemManager::ReadFileFromVPKHook(VPKData* vpkInfo, __int32* b, const char* filename)
{
    // If the path is one of our replacements, we will not allow the read from the VPK to happen
    if (ShouldReplaceFile(filename))
    {
        SPDLOG_DEBUG(m_logger, "ReadFileFromVPK: blocking response for {} from {}", filename, vpkInfo->path);
        *b = -1;
        return b;
    }

    FileHandle_t result = ReadFileFromVPK(vpkInfo, b, filename);
    SPDLOG_TRACE(m_logger, "ReadFileFromVPK: vpk = {}, file = {}, result = {}", vpkInfo->path, filename, *b);

    if (*b != -1)
    {
        std::string strVPK(vpkInfo->path);
        std::smatch m;
        std::regex_search(strVPK, m, s_mapFromVPKRegex);
        if (!m.empty())
        {
            m_lastMapReadFrom = m[1];
        }
    }

    return result;
}

// TODO: If we have mounted other VPKs and we unload the DLL, should we unmount them?
VPKData* FileSystemManager::MountVPKHook(IFileSystem* fileSystem, const char* vpkPath)
{
    SPDLOG_DEBUG(m_logger, "IFileSystem::MountVPK: vpkPath = {}", vpkPath);
    VPKData* res = IFileSystem_MountVPK(fileSystem, vpkPath);

    // When a level is loaded, the VPK for the map is mounted, so we'll mount every
    // other map's VPK at the same time.
    // TODO: This might be better moved to a hook on the function that actually loads up the map?
    for (const auto& otherMapVPK : m_mapVPKs)
    {
        if (otherMapVPK != vpkPath)
        {
            SPDLOG_DEBUG(m_logger, "Mounting VPK: {}", otherMapVPK);
            VPKData* injectedRes = IFileSystem_MountVPK(fileSystem, otherMapVPK.c_str());
            if (injectedRes == nullptr)
            {
                m_logger->error("Failed to mount VPK: {} (was mounting: {})", otherMapVPK, vpkPath);
            }
        }
    }

    return res;
}

const std::vector<std::string>& FileSystemManager::GetMapNames()
{
    return m_mapNames;
}

const std::string& FileSystemManager::GetLastMapReadFrom()
{
    return m_lastMapReadFrom;
}

bool FileSystemManager::ShouldReplaceFile(const std::string& path)
{
    // TODO: See if this is worth optimising by keeping a map in memory of the available files
    std::ifstream f(m_compiledPath + path);
    return f.good();
}

void FileSystemManager::DumpFile(FileHandle_t handle, const std::string& dir, const std::string& path)
{
    if (handle == nullptr)
    {
        return;
    }
    
    std::experimental::filesystem::create_directories(m_dumpPath + dir);
    std::ofstream f(m_dumpPath + path, std::ios::binary);
    char data[4096];

    int totalBytes = 0;
    int readBytes = 0;
    do
    {
        readBytes = m_engineFileSystem->m_vtable2->Read(&m_engineFileSystem->m_vtable2, data, std::size(data), handle);
        f.write(data, readBytes);
        totalBytes += readBytes;
    }
    while (readBytes == std::size(data));

    SPDLOG_TRACE(m_logger, "Wrote {} bytes to {}", totalBytes, path);
}

void FileSystemManager::DumpVPKScripts(const std::string& vpkPath)
{
    SPDLOG_TRACE(m_logger, "Dumping from {}", vpkPath);
    VPKData* result = IFileSystem_MountVPK(m_engineFileSystem, vpkPath.c_str());
    if (result == nullptr)
    {
        m_logger->error("Failed to dump scripts from {}", vpkPath);
        return;
    }

    for (int i = 0; i < result->numEntries; i++)
    {
        // Only process files in scripts
        if (strncmp(result->entries[i].directory, "scripts", 7) != 0)
        {
            continue;
        }

        // TODO: Better error handling here
        std::string path = fmt::format("{}/{}.{}", result->entries[i].directory, result->entries[i].filename, result->entries[i].extension);
        Util::FindAndReplaceAll(path, "\\", "/");
        SPDLOG_TRACE(m_logger, "Dumping {}", path);
        FileHandle_t handle = m_engineFileSystem->m_vtable2->Open(&m_engineFileSystem->m_vtable2, path.c_str(), "rb", "GAME", 0);
        SPDLOG_TRACE(m_logger, "Handle = {}", handle);
        DumpFile(handle, result->entries[i].directory, path);
        m_engineFileSystem->m_vtable2->Close(m_engineFileSystem, handle);
    }
}
