#include "stdafx.h"

SigScanFunc<void, ConCommand*, const char*, void(*)(const CCommand&), const char*, int, void*> ConCommand_ConCommand("engine.dll", "\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\x45\x33\xD2", "xxxxx?xxxxxx");

ConCommandManager::ConCommandManager()
{
    m_logger = spdlog::get("logger");
}

void ConCommandManager::RegisterCommand(const char* name, void(*callback)(const CCommand&), const char* helpString, int flags)
{
    m_logger->info("Adding console command: {}", name);
    m_commands.emplace_back();
    ConCommand* newCommand = &m_commands.back();
    ConCommand_ConCommand(newCommand, name, callback, helpString, flags, nullptr);
}

void ConCommandManager::ExecuteCommand(const std::string& commandStr)
{
    auto& engineClient = SDK().GetEngineClient();
    engineClient->m_vtable->ClientCmd_Unrestricted(engineClient, commandStr.c_str());
}

ConCommandManager::~ConCommandManager()
{
    // TODO: Release all commands
}
