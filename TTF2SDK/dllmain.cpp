#include "stdafx.h"
#include <iostream>

std::unique_ptr<TTF2SDK> SetupSDK()
{
    std::unique_ptr<Console> console;
    std::unique_ptr<Logger> logger;

    // Separate try catch because these are required for logging to work
    try
    {
        console = std::make_unique<Console>();
        logger = std::make_unique<Logger>("TTF2SDK.log");
    }
    catch (std::exception)
    {
        return nullptr;
    }
    
    try
    {
        return std::make_unique<TTF2SDK>(std::move(console), std::move(logger));
    }
    catch (std::exception& ex)
    {
        logger->Get().critical("Failed to initialise SDK: {}", ex.what());
        return nullptr;
    }
}

std::unique_ptr<TTF2SDK> g_SDK;

DWORD WINAPI OnAttach(LPVOID lpThreadParameter)
{
    // Setup the SDK or unload the DLL if we can't
    g_SDK = SetupSDK();
    if (g_SDK == nullptr)
    {
        FreeLibraryAndExitThread((HMODULE)lpThreadParameter, 0);
        return 0;
    }

    g_SDK->Logger().info("Titanfall 2 SDK loaded");

    // Process input
    while (true)
    {
        std::string input;
        std::cin >> input;

        if (input == "unload")
        {
            g_SDK->Logger().info("Unloading SDK");
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
