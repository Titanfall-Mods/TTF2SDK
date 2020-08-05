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
    std::vector<std::string> m_customAssets; // Paths to custom assets, i.e. something not already in the engine and patchable
    std::vector<CustomScriptInfo> m_customScripts;

    friend class ModManager;
};

class ModManager
{
public:
    ModManager(ConCommandManager& conCommandManager);
    void ReloadModsCommand(const CCommand& args);
    void CompileMods();

private:
    void PatchFile(const std::string& gamePath, const std::vector<fs::path>& patchFiles);

    std::shared_ptr<spdlog::logger> m_logger;
    std::list<Mod> m_mods;
};
