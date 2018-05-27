#include "stdafx.h"
#include <iostream>
#include <functional>
#include <string>

const int MAX_SIG_SCAN_REGISTRATIONS = 100;
SDKSettings g_settings;
std::atomic_flag g_initialised = ATOMIC_FLAG_INIT;

DWORD WINAPI OnAttach(LPVOID lpThreadParameter)
{
    // Setup the SDK or unload the DLL if we can't
    if (!SetupSDK(g_settings))
    {
        FreeLibraryAndExitThread((HMODULE)lpThreadParameter, 0);
        return 0;
    }

    auto logger = spdlog::get("logger");
    logger->info("Titanfall 2 SDK loaded");

    // Process input
    std::string input;
    while (std::getline(std::cin, input))
    {
        if (input == "unload")
        {
            logger->info("Unloading SDK");
            break;
        }
        else
        {
            SDK().GetConCommandManager().ExecuteCommand(input);
        }
    }

    FreeSDK();
    FreeLibraryAndExitThread((HMODULE)lpThreadParameter, 0);
    return 0;
}

extern "C" __declspec(dllexport) DWORD InitialiseSDK(LPVOID lpParameter)
{
    if (g_initialised.test_and_set(std::memory_order_acquire))
    {
        return 1;
    }

    // The launcher will pass the configuration settings through lpParameter parameter as a struct
    SDKSettings* args = reinterpret_cast<SDKSettings*>(lpParameter);
    SDKSettings newSettings(*args);
    g_settings = newSettings;
    CreateThread(NULL, 0, OnAttach, NULL, 0, NULL);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
