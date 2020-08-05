#include "stdafx.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/filereadstream.h>

#pragma warning( push )  
#pragma warning( disable : 4267 )   
#include "diff_match_patch.h"
#pragma warning( pop )   

ModManager& ModMan()
{
    return SDK().GetModManager();
}

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&ModManager::##name), &ModManager::##name, decltype(&ModMan), &ModMan>::Call

const char* MOD_INFO_SCHEMA = R"END(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "description": "JSON schema for TTF2SDK modV1.json", 
    "type": "object",
    "properties": {
        "Name": {
            "type": "string"
        },
        "Description": {
            "type": "string"
        },
        "Authors": {
            "type": "array",
            "items": {
                "type": "string"
            }
        },
        "Contacts": {
            "type": "array",
            "items": {
                "type": "string"
            }
        },
        "Version": {
            "type": "string"
        },
        "CustomScripts": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "Path": {
                        "type": "string"
                    },
                    "RunOn": {
                        "type": "string"
                    },
                    "RunBefore": {
                        "type": "string"
                    },
                    "RunAfter": {
                        "type": "string"
                    },
                    "ServerCallback": {
                        "type": "string"
                    },
                    "ClientCallback": {
                        "type": "string"
                    }
                },
                "oneOf": [{
                        "required": ["RunOn"]
                    }, {
                        "required": ["RunBefore"]
                    }, {
                        "required": ["RunAfter"]
                    }
                ],
                "required": ["Path"]
            }
        }
    },
    "required": ["Name"]
}
)END";

rapidjson::SchemaDocument GetModV1Schema()
{
    rapidjson::Document d;
    d.Parse(MOD_INFO_SCHEMA);
    if (d.HasParseError())
    {
        throw std::exception("Failed to parse ModV1 JSON schema");
    }

    rapidjson::SchemaDocument sd(d);
    return sd;
}

rapidjson::Document GetModDocument(std::ifstream& f)
{
    static rapidjson::SchemaDocument sd = GetModV1Schema();
    rapidjson::SchemaValidator validator(sd);
    std::string jsonData = Util::ReadFileToString(f);
    rapidjson::Document d;
    if (d.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(jsonData.c_str(), jsonData.length()).HasParseError())
    {
        throw std::exception("Failed to parse mod.json - ensure it is a valid JSON file");
    }

    if (!d.Accept(validator))
    {
        auto logger = spdlog::get("logger");
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        logger->error("Invalid schema: {}", sb.GetString());
        logger->error("Invalid keyword: {}", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        logger->error("Invalid document: {}", sb.GetString());

        throw std::exception("mod.json was not of the correct format, see console for details");
    }

    return d;
}

void CreateCustomScriptInfo(CustomScriptInfo& script, const fs::path& modFolder, const rapidjson::Value& info)
{
    // Ensure that the path is valid
    // TODO: Ensure that it's all forward slashes
    // TODO: Ensure it does not start with a slash
    script.Path = info["Path"].GetString();

    {
        fs::path fullPath = modFolder / "scripts" / "vscripts" / script.Path;
        std::ifstream f(fullPath);
        if (!f.good())
        {
            throw std::runtime_error(fmt::format("Failed to find or read from custom script with path {}", script.Path));
        }
    }
    
    std::string relativePath = "scripts/vscripts/" + script.Path;
    if (SDK().GetFSManager().FileExists(relativePath.c_str(), "GAME"))
    {
        throw std::runtime_error(fmt::format("Custom script with path {} already exists in the game files", script.Path));
    }

    // Figure out what kind of trigger we're using
    if (info.HasMember("RunOn"))
    {
        script.RunTrigger = info["RunOn"].GetString();
        script.TriggerType = RUN_WHEN;
    }
    else if (info.HasMember("RunAfter"))
    {
        script.RunTrigger = info["RunAfter"].GetString();
        script.TriggerType = RUN_AFTER;
    }
    else if (info.HasMember("RunBefore"))
    {
        script.RunTrigger = info["RunBefore"].GetString();
        script.TriggerType = RUN_BEFORE;
    }
    else
    {
        throw std::runtime_error(fmt::format("No valid trigger was specified for custom script with path {}", script.Path));
    }

    if (info.HasMember("ServerCallback"))
    {
        script.ServerCallback = info["ServerCallback"].GetString();
    }

    if (info.HasMember("ClientCallback"))
    {
        script.ClientCallback = info["ClientCallback"].GetString();
    }
}

Mod::Mod(const fs::path& modFolder) 
    : m_folder(modFolder)
{
    auto logger = spdlog::get("logger");

    // Read mod.json
    fs::path modJsonPath = m_folder / "mod.json";
    std::ifstream modJsonFile(modJsonPath);
    if (!modJsonFile.good())
    {
        throw std::exception("Failed to find or read from mod.json - are you sure it exists?");
    }

    // Pull details out of the mod document
    rapidjson::Document d = GetModDocument(modJsonFile);
    m_name = d["Name"].GetString();
    m_description = d.HasMember("Description") ? d["Description"].GetString() : "";

    if (d.HasMember("Authors"))
    {
        const rapidjson::Value& authors = d["Authors"];
        for (rapidjson::Value::ConstValueIterator itr = authors.Begin(); itr != authors.End(); itr++)
        {
            m_authors.emplace_back(itr->GetString());
        }
    }

    if (d.HasMember("Contacts"))
    {
        const rapidjson::Value& contacts = d["Contacts"];
        for (rapidjson::Value::ConstValueIterator itr = contacts.Begin(); itr != contacts.End(); itr++)
        {
            m_contacts.emplace_back(itr->GetString());
        }
    }

    m_version = d.HasMember("Version") ? d["Version"].GetString() : "";

    // Fill in all of the specified custom scripts
    // TODO: Check that the user hasn't specified a custom script twice
    std::unordered_set<std::string> customPaths;
    if (d.HasMember("CustomScripts"))
    {
        const rapidjson::Value& customScripts = d["CustomScripts"];
        m_customScripts.resize(customScripts.Size());
        customPaths.reserve(customScripts.Size());
        fs::path basePath = m_folder / "scripts/vscripts";
        for (rapidjson::SizeType i = 0; i < customScripts.Size(); i++)
        {
            CreateCustomScriptInfo(m_customScripts[i], m_folder, customScripts[i]);
            customPaths.insert((basePath / m_customScripts[i].Path).string());
        }
    }

    // Iterate over all the files in the mod, putting relevant files into patches and customs
    std::string scriptsFolder = (m_folder / "scripts").string();
    std::string mpLevelsFolder = (m_folder / "scripts/vscripts/mp/levels").string();
    for (auto& dirIter : fs::recursive_directory_iterator(m_folder))
    {
        if (dirIter.status().type() == fs::file_type::directory)
        {
            continue;
        }

        fs::path path = dirIter.path();

        // If the path is inside the scripts folder, need to do some special processing
        std::string pathString = path.string();
        std::string relative = pathString.substr(m_folder.string().size() + 1);
        if (!pathString.compare(0, scriptsFolder.size(), scriptsFolder) && 
            pathString.compare(0, mpLevelsFolder.size(), mpLevelsFolder))
        {
            // If it's already marked as a custom file, move on
            if (customPaths.find(pathString) != customPaths.end())
            {
                continue;
            }

            // Check if the path exists in the engine
            if (!SDK().GetFSManager().FileExists(relative.c_str(), "GAME"))
            {
                logger->warn("{} in {} is not a custom script and does not correspond to a file in the game - it will not be loaded", relative, m_folder);
                continue;
            }

            // Add the file as a patch
            m_filesToPatch.emplace_back(relative);
        }
        else if (path.parent_path() != m_folder) // Don't add assets in the root folder
        {
            // Add the file as a custom asset
            m_customAssets.emplace_back(relative);
        }
    }
}

ModManager::ModManager(ConCommandManager& conCommandManager)
{
    m_logger = spdlog::get("logger");
    conCommandManager.RegisterCommand("reload_mods", WRAPPED_MEMBER(ReloadModsCommand), "Reload all mods", 0);
}

void ModManager::ReloadModsCommand(const CCommand& args)
{
    try
    {
        CompileMods();
    }
    catch (std::exception& e)
    {
        m_logger->error("Failed to reload mods: {}", e.what());
    }
}

void ModManager::CompileMods()
{
    m_logger->info("Compiling mods...");

    FileSystemManager& filesystem = SDK().GetFSManager();
    SquirrelManager& sq = SDK().GetSQManager();
    const fs::path& compilePath = filesystem.GetCompilePath();

    // Clear out current mods
    m_mods.clear();

    // Clear all the callbacks
    sq.ClearCallbacks();

    // Clear out the compiled assets folder
    fs::remove_all(compilePath);
    fs::create_directories(compilePath);

    // Mount all the VPKs
    filesystem.MountAllVPKs();

    // Iterate over all the directories in the mods path and create the Mod object for them, then compile the mod
    std::string scriptsRson = filesystem.ReadOriginalFile("scripts/vscripts/scripts.rson", "GAME");
    std::unordered_map<std::string, std::vector<fs::path>> filesToPatch; // relative path => [ absolute path on disk ]

    for (auto& p : fs::directory_iterator(filesystem.GetModsPath()))
    {
        if (p.status().type() != fs::file_type::directory)
        {
            continue;
        }

        try
        {
            // Load the mod details
            Mod mod(p.path());

            // Copy the custom assets to the compiled directory
            for (auto& customPath : mod.m_customAssets)
            {
                SPDLOG_TRACE(m_logger, "Copying custom asset {} from {}", customPath, mod.m_folder);
                fs::path destFile = compilePath / customPath;
                if (fs::exists(destFile))
                {
                    throw std::runtime_error(fmt::format("{} already exists as a custom asset in another mod", customPath));
                }

                fs::path destFolder = destFile.remove_filename();
                fs::create_directories(destFolder);
                fs::copy(mod.m_folder / customPath, destFolder);
            }

            // Add all the details to our patched scripts.rson and copy the script to the compiled dir
            // TODO: Also add the ability to add custom scripts to stuff that isn't in scripts.rson
            std::string newScriptsRson(scriptsRson);
            for (const auto& customScript : mod.m_customScripts)
            {
                std::string customPath = "scripts/vscripts/" + customScript.Path;
                SPDLOG_TRACE(m_logger, "Copying custom script {} from {}", customPath, mod.m_folder);
                fs::path destFolder = (compilePath / customPath).remove_filename();
                fs::create_directories(destFolder);
                fs::copy(mod.m_folder / customPath, destFolder);

                if (customScript.TriggerType == RUN_AFTER)
                {
                    Util::FindAndReplaceAll(newScriptsRson, customScript.RunTrigger + "\r\n", customScript.RunTrigger + "\r\n\t" + customScript.Path + "\r\n");
                }
                else if (customScript.TriggerType == RUN_BEFORE)
                {
                    Util::FindAndReplaceAll(newScriptsRson, customScript.RunTrigger + "\r\n", customScript.Path + "\r\n\t" + customScript.RunTrigger + "\r\n");
                }
                else if (customScript.TriggerType == RUN_WHEN)
                {
                    newScriptsRson += "\r\nWhen: \"" + customScript.RunTrigger + "\"\r\nScripts:\r\n[\r\n\t" + customScript.Path + "\r\n]\r\n";
                }

                if (!customScript.ClientCallback.empty())
                {
                    sq.AddClientCallback(customScript.ClientCallback);
                }
                
                if (!customScript.ServerCallback.empty())
                {
                    sq.AddServerCallback(customScript.ServerCallback);
                }
            }

            // Add to the list of files to get patched
            for (const auto& patchFile : mod.m_filesToPatch)
            {
                filesToPatch[patchFile].push_back(mod.m_folder / patchFile);
            }

            m_mods.push_back(mod);
            scriptsRson = std::move(newScriptsRson);
            m_logger->info("Mod loaded: {}", mod.m_name);
        }
        catch (std::exception& e)
        {
            m_logger->error("Failed to load mod from {}: {}", p.path().filename(), e.what());
        }
    }

    // Write patched scripts.rson to compiled dir
    fs::path destFolder = compilePath / "scripts/vscripts";
    fs::create_directories(destFolder);
    {
        std::ofstream f(destFolder / "scripts.rson", std::ios::binary);
        f << scriptsRson;
    }

    // Patch the remaining game files
    for (const auto& kv : filesToPatch)
    {
        try
        {
            PatchFile(kv.first, kv.second);
        }
        catch (std::exception& e)
        {
            m_logger->error("Failed to patch {}: {}", kv.first, e.what());
        }
    }

    m_logger->info("{} mods loaded", m_mods.size());
}

std::string MergeFile(const std::string& currentData, const std::string& baseData, const fs::path& patchFile)
{
    std::ifstream f(patchFile);
    if (!f.good())
    {
        throw std::runtime_error("Failed to open patch file");
    }

    std::string fileData;
    f.seekg(0, std::ios::end);
    fileData.reserve(f.tellg());
    f.seekg(0, std::ios::beg);

    fileData.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    Util::FindAndReplaceAll(fileData, "\r\n", "\n");

    diff_match_patch<std::string> dmp;
    dmp.Diff_Timeout = 0.0;
    
    auto diffs = dmp.diff_main(baseData, fileData, false);
    auto patches = dmp.patch_make(diffs);
    auto patchResult = dmp.patch_apply(patches, currentData);

    for (bool success : patchResult.second)
    {
        if (!success)
        {
            throw std::runtime_error("Failed to apply patch");
        }
    }

    return std::move(patchResult.first);
}

void ModManager::PatchFile(const std::string& gamePath, const std::vector<fs::path>& patchFiles)
{
    const fs::path& compilePath = SDK().GetFSManager().GetCompilePath();

    // Create directories for file in output
    fs::create_directories((compilePath / gamePath).remove_filename());

    // Read the orignial file data
    std::string baseData = SDK().GetFSManager().ReadOriginalFile(gamePath.c_str(), "GAME");
    Util::FindAndReplaceAll(baseData, "\r\n", "\n");
    std::string currentData(baseData);

    // Apply all the patches
    for (const fs::path& patchFile : patchFiles)
    {
        try
        {
            SPDLOG_DEBUG(m_logger, "Merging {} into {}", patchFile, gamePath);
            currentData = std::move(MergeFile(currentData, baseData, patchFile));
        }
        catch (std::exception& e)
        {
            m_logger->error("Failed to merge {} into {}: {}", patchFile, gamePath, e.what());
        }
    }

    // Write the data out
    std::ofstream f(compilePath / gamePath, std::ios::binary);
    f << currentData;
}
