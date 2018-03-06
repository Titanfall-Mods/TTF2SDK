#include "stdafx.h"
#include <iostream>

DWORD WINAPI OnAttach(LPVOID lpThreadParameter)
{
    // Setup the SDK or unload the DLL if we can't
    if (!SetupSDK())
    {
        FreeLibraryAndExitThread((HMODULE)lpThreadParameter, 0);
        return 0;
    }

    SDK().Logger().info("Titanfall 2 SDK loaded");

    // Process input
    while (true)
    {
        std::string input;
        std::cin >> input;

        if (input == "unload")
        {
            SDK().Logger().info("Unloading SDK");
            break;
        }
    }

    FreeLibraryAndExitThread((HMODULE)lpThreadParameter, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, OnAttach, hModule, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
