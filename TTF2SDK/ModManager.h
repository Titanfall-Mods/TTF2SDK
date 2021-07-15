#pragma once

enum ScriptRunTrigger
{
    RUN_WHEN,
    RUN_BEFORE,
    RUN_AFTER
};

struct CustomScriptInfo
{
    std::string Path; // Relative path from scripts/vscripts
    ScriptRunTrigger TriggerType;
    std::string RunTrigger;
    std::string ServerCallback;
    std::string ClientCallback;
};

struct GamemodeInfo
{
    bool HasGamemode;
    std::string Id;
    std::string Name;
    std::string Description;
};

class Mod
{
public:
    Mod(const fs::path& modFolder);

private:
    fs::path m_folder;
    std::string m_name;
    std::string m_description;
    std::vector<std::string> m_authors;
    std::vector<std::string> m_contacts;
    std::string m_version;

    std::vector<std::string> m_filesToPatch; // Relative paths to assets which can be patched into the game
    std::vector<std::string>
        m_customAssets; // Paths to custom assets, i.e. something not already in the engine and patchable
    std::vector<CustomScriptInfo> m_customScripts;
    GamemodeInfo m_gamemode;

    friend class ModManager;
};

class ModManager
{
public:
    ModManager(ConCommandManager& conCommandManager, SquirrelManager& sqManager);
    void ReloadModsCommand(const CCommand& args);
    void CompileMods();

    SQInteger GetIcepickGamemodes_Client(HSQUIRRELVM v);
    SQInteger GetIcepickGamemodes_Server(HSQUIRRELVM v);
    std::string& GetCurrentGamemode()
    {
        return m_gamemode;
    }
    SQInteger SqGetCurrentGamemode(HSQUIRRELVM v);
    void SetGamemodeCommand(const CCommand& args);

private:
    void PatchFile(const std::string& gamePath, const std::vector<fs::path>& patchFiles);

    std::shared_ptr<spdlog::logger> m_logger;
    std::list<Mod> m_mods;
    std::string m_gamemode;
};
