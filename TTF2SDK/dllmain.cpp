#include "stdafx.h"
#include <iostream>
#include <functional>
#include <string>

const int MAX_SIG_SCAN_REGISTRATIONS = 100;

DWORD WINAPI OnAttach(LPVOID lpThreadParameter)
{
    // Setup the SDK or unload the DLL if we can't
    if (!SetupSDK())
    {
        FreeLibraryAndExitThread((HMODULE)lpThreadParameter, 0);
        return 0;
    }

    auto logger = spdlog::get("logger");
    logger->info("Titanfall 2 SDK loaded");

    // Process input
    while (true)
    {
        std::string input;
        std::cin >> input;

        if (input == "unload")
        {
            logger->info("Unloading SDK");
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
