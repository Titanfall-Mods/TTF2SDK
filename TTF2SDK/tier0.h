#pragma once

extern "C"
{
    IMemAlloc* CreateGlobalMemAlloc();
    __declspec(dllimport) extern IMemAlloc* g_pMemAllocSingleton;
    bool ThreadInMainThread();
}
