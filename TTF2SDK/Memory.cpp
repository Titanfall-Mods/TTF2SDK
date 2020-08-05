#include "stdafx.h"

void* operator new(std::size_t n)
{
    if (!g_pMemAllocSingleton)
    {
        g_pMemAllocSingleton = CreateGlobalMemAlloc();
    }

    return g_pMemAllocSingleton->m_vtable->Alloc(g_pMemAllocSingleton, n);
}

void operator delete(void* p) throw()
{
    if (!g_pMemAllocSingleton)
    {
        g_pMemAllocSingleton = CreateGlobalMemAlloc();
    }

    g_pMemAllocSingleton->m_vtable->Free(g_pMemAllocSingleton, p);
}
