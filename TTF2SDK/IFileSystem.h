#pragma once

enum SearchPathAdd_t
{
    PATH_ADD_TO_HEAD,		// First path searched
    PATH_ADD_TO_TAIL,		// Last path searched
};

class CSearchPath
{
public:
    unsigned char unknown[0x18];
    const char* debugPath;
};

class IFileSystem
{
public:
    struct VTable
    {
        void* unknown[10];
        void(*AddSearchPath) (IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType);
        void* unknown2[84];
        bool(*ReadFromCache) (IFileSystem* fileSystem, const char* path, void* result);
        void* unknown3[15];
        unsigned int*(*MountVPK) (IFileSystem* fileSystem, const char* vpkPath);
    };

    struct VTable2
    {
        void* unknown[14];
        bool(*ReadFile) (IFileSystem* fileSystem, const char* pFileName, const char* pPath, void* buf, int64_t nMaxBytes, int64_t nStartingByte, void* pfnAlloc);
    };

    VTable* m_vtable;
    VTable2* m_vtable2;
};
