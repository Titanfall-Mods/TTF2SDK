#pragma once

class IMemAlloc
{
public:
    struct VTable
    {
        void* unknown[1];
        void*(*Alloc) (IMemAlloc* memAlloc, size_t nSize);
        void* unknown2[3];
        void(*Free) (IMemAlloc* memAlloc, void* pMem);
    };

    VTable* m_vtable;
};

extern "C"
{
    IMemAlloc* CreateGlobalMemAlloc();
    __declspec(dllimport) extern IMemAlloc* g_pMemAllocSingleton;
    bool ThreadInMainThread(); // TODO: Maybe move all of these into tier0.h, including ones in memory.h
}

void* operator new(std::size_t n);
void operator delete(void* p) throw();
