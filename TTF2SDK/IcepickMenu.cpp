#include "stdafx.h"
#include "ModelsList.h"

IcepickMenu& Menu()
{
    return SDK().GetIcepickMenu();
}

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&IcepickMenu::##name), &IcepickMenu::##name, decltype(&Menu), &Menu>::Call

IcepickMenu::IcepickMenu(ConCommandManager& conCommandManager, UIManager& uiManager, SquirrelManager& sqManager)
{
    // Commands
    conCommandManager.RegisterCommand("show_icepick_menu", WRAPPED_MEMBER(ShowMenuCommand), "Shows the Icepick Menu", 0);
    conCommandManager.RegisterCommand("hide_icepick_menu", WRAPPED_MEMBER(HideMenuCommand), "Hides the Icepick Menu", 0);
    conCommandManager.RegisterCommand("register_tool", WRAPPED_MEMBER(RegisterToolCommand), "Register a tool to appear in the spawn menu", 0);
    conCommandManager.RegisterCommand("register_option", WRAPPED_MEMBER(RegisterToolOption), "Register an option for a specific tool", 0);
    conCommandManager.RegisterCommand("option_set_default", WRAPPED_MEMBER(SetOptionDefaultValue), "Set the default value for an option", 0);
    conCommandManager.RegisterCommand("option_set_minmax", WRAPPED_MEMBER(SetOptionMinMax), "Set the minmax values for an option", 0);

    // Add models
    m_ModelsList = new ModelsList();
    m_ViewingDirectory = &m_ModelsList->BaseDir;

    // Add Entities
    EntityCategory entsHumans = EntityCategory("Humans", SpawnlistTab::Entities, "Spawnmenu_SpawnNpc(\"{0}\")");
    entsHumans.Ents.push_back(SpawnEntity("Rifle Grunt", "npc_soldier"));
    entsHumans.Ents.push_back(SpawnEntity("Shotgun Grunt", "npc_soldier_shotgun"));
    entsHumans.Ents.push_back(SpawnEntity("SMG Grunt", "npc_soldier_smg"));
    m_EntCategories.push_back(entsHumans);

    EntityCategory entsRobots = EntityCategory("Robots", SpawnlistTab::Entities, "Spawnmenu_SpawnNpc(\"{0}\")");
    entsRobots.Ents.push_back(SpawnEntity("Spectre", "npc_spectre"));
    entsRobots.Ents.push_back(SpawnEntity("Stalker", "npc_stalker"));
    entsRobots.Ents.push_back(SpawnEntity("Zombie Stalker", "npc_stalker_zombie"));
    entsRobots.Ents.push_back(SpawnEntity("Zombie Stalker (Mossy)", "npc_stalker_zombie_mossy"));
    entsRobots.Ents.push_back(SpawnEntity("Reaper", "npc_super_spectre"));
    entsRobots.Ents.push_back(SpawnEntity("Drone", "npc_drone"));
    entsRobots.Ents.push_back(SpawnEntity("Rocket Drone", "npc_drone_rocket"));
    entsRobots.Ents.push_back(SpawnEntity("Plasma Drone", "npc_drone_plasma"));
    entsRobots.Ents.push_back(SpawnEntity("Worker Drone", "npc_drone_worker"));
    entsRobots.Ents.push_back(SpawnEntity("Tick", "npc_frag_drone"));
    entsRobots.Ents.push_back(SpawnEntity("Marvin", "npc_marvin"));
    m_EntCategories.push_back(entsRobots);

    EntityCategory entsTitans = EntityCategory("Titans", SpawnlistTab::Entities, "Spawnmenu_SpawnNpc(\"{0}\")");
    entsTitans.Ents.push_back(SpawnEntity("BT-7274", "npc_titan_bt"));
    entsTitans.Ents.push_back(SpawnEntity("BT-7274 2", "npc_titan_bt_spare"));
    entsTitans.Ents.push_back(SpawnEntity("Atlas", "npc_titan_atlas"));
    entsTitans.Ents.push_back(SpawnEntity("Stryder", "npc_titan_stryder"));
    entsTitans.Ents.push_back(SpawnEntity("Ogre", "npc_titan_ogre"));
    m_EntCategories.push_back(entsTitans);

    // Add Weapons
    EntityCategory tools = EntityCategory("Tools", SpawnlistTab::Weapons, "Spawnmenu_GiveWeapon(\"{0}\")");
    tools.Ents.push_back(SpawnEntity("Toolgun", "mp_weapon_shotgun_pistol"));
    m_EntCategories.push_back(tools);

    EntityCategory pilotPrimaries = EntityCategory("Primaries - Pilot", SpawnlistTab::Weapons, "Spawnmenu_GiveWeapon(\"{0}\")");
    pilotPrimaries.Ents.push_back(SpawnEntity("R-201 Carbine", "mp_weapon_rspn101"));
    pilotPrimaries.Ents.push_back(SpawnEntity("R-101 Carbine", "mp_weapon_rspn101_og"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Hemlok BF-R", "mp_weapon_hemlok"));
    pilotPrimaries.Ents.push_back(SpawnEntity("G2A5", "mp_weapon_g2"));
    pilotPrimaries.Ents.push_back(SpawnEntity("V-47 Flatline", "mp_weapon_vinson"));
    pilotPrimaries.Ents.push_back(SpawnEntity("CAR", "mp_weapon_car"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Alternator", "mp_weapon_alternator_smg"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Volt", "mp_weapon_hemlok_smg"));
    pilotPrimaries.Ents.push_back(SpawnEntity("R-47", "mp_weapon_r97"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Spitfire", "mp_weapon_lmg"));
    pilotPrimaries.Ents.push_back(SpawnEntity("L_STAR", "mp_weapon_lstar"));
    pilotPrimaries.Ents.push_back(SpawnEntity("X-55 Devotion", "mp_weapon_esaw"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Kraber-AP Sniper", "mp_weapon_sniper"));
    pilotPrimaries.Ents.push_back(SpawnEntity("D-2 Double Take", "mp_weapon_doubletake"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Longbow-DMR", "mp_weapon_dmr"));
    pilotPrimaries.Ents.push_back(SpawnEntity("EVA-8 Auto", "mp_weapon_shotgun"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Mastiff", "mp_weapon_mastiff"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Sidewinder SMR", "mp_weapon_smr"));
    pilotPrimaries.Ents.push_back(SpawnEntity("EPG-1", "mp_weapon_epg"));
    pilotPrimaries.Ents.push_back(SpawnEntity("R-6P Softball", "mp_weapon_softball"));
    pilotPrimaries.Ents.push_back(SpawnEntity("EM-4 Cold War", "mp_weapon_pulse_lmg"));
    pilotPrimaries.Ents.push_back(SpawnEntity("Wingman Elite", "mp_weapon_wingman_n"));
    pilotPrimaries.Ents.push_back(SpawnEntity("SA-3 Mozambique", "mp_weapon_shotgun_pistol"));
    m_EntCategories.push_back(pilotPrimaries);

    EntityCategory pilotSecondaries = EntityCategory("Secondaries - Pilot", SpawnlistTab::Weapons, "Spawnmenu_GiveWeapon(\"{0}\")");
    pilotSecondaries.Ents.push_back(SpawnEntity("Hammond P2016", "mp_weapon_semipistol"));
    pilotSecondaries.Ents.push_back(SpawnEntity("RE-45 Auto", "mp_weapon_autopistol"));
    pilotSecondaries.Ents.push_back(SpawnEntity("B3 Wingman", "mp_weapon_wingman"));
    pilotSecondaries.Ents.push_back(SpawnEntity("Charge Rifle", "mp_weapon_defender"));
    pilotSecondaries.Ents.push_back(SpawnEntity("MGL Mag Launcher", "mp_weapon_mgl"));
    pilotSecondaries.Ents.push_back(SpawnEntity("LG-97 Thunderbolt", "mp_weapon_arc_launcher"));
    pilotSecondaries.Ents.push_back(SpawnEntity("Archer", "mp_weapon_rocket_launcher"));
    m_EntCategories.push_back(pilotSecondaries);

    EntityCategory pilotAbilities = EntityCategory("Pilot Abilities", SpawnlistTab::Weapons, "Spawnmenu_GiveAbility(\"{0}\")");
    pilotAbilities.Ents.push_back(SpawnEntity("Cloak", "mp_ability_cloak"));
    pilotAbilities.Ents.push_back(SpawnEntity("Grapple", "mp_ability_grapple"));
    pilotAbilities.Ents.push_back(SpawnEntity("Stim", "mp_ability_heal"));
    pilotAbilities.Ents.push_back(SpawnEntity("Holopilot", "mp_ability_holopilot"));
    pilotAbilities.Ents.push_back(SpawnEntity("Phase Shift", "mp_ability_shifter"));
    pilotAbilities.Ents.push_back(SpawnEntity("Pulse Blade", "mp_weapon_grenade_sonar"));
    pilotAbilities.Ents.push_back(SpawnEntity("A-Wall", "mp_weapon_deployable_cover"));
    pilotAbilities.Ents.push_back(SpawnEntity("Timeshift [Effect and Cause]", "mp_ability_timeshift"));
    pilotAbilities.Ents.push_back(SpawnEntity("Arcblast [Dev]", "mp_ability_arc_blast"));
    pilotAbilities.Ents.push_back(SpawnEntity("Super Phase Shift [Dev]", "mp_ability_shifter_super"));
    m_EntCategories.push_back(pilotAbilities);

    EntityCategory pilotGrenades = EntityCategory("Pilot Ordnance", SpawnlistTab::Weapons, "Spawnmenu_GiveGrenade(\"{0}\")");
    pilotGrenades.Ents.push_back(SpawnEntity("Frag Grenade", "mp_weapon_frag_grenade"));
    pilotGrenades.Ents.push_back(SpawnEntity("Firestar", "mp_weapon_thermite_grenade"));
    pilotGrenades.Ents.push_back(SpawnEntity("Electric Smoke Grenade", "mp_weapon_grenade_electric_smoke"));
    pilotGrenades.Ents.push_back(SpawnEntity("Arc Grenade", "mp_weapon_grenade_emp"));
    pilotGrenades.Ents.push_back(SpawnEntity("Gravity Star", "mp_weapon_grenade_gravity"));
    pilotGrenades.Ents.push_back(SpawnEntity("Satchel", "mp_weapon_satchel"));
    m_EntCategories.push_back(pilotGrenades);

    EntityCategory pilotMelee = EntityCategory("Pilot Melee", SpawnlistTab::Weapons, "Spawnmenu_GiveMelee(\"{0}\")");
    pilotMelee.Ents.push_back(SpawnEntity("Standard Melee", "melee_pilot_emptyhanded"));
    pilotMelee.Ents.push_back(SpawnEntity("Sword Melee", "melee_pilot_sword"));
    m_EntCategories.push_back(pilotMelee);

    uiManager.AddDrawCallback("IcepickMenu", std::bind(&IcepickMenu::DrawCallback, this));

    sqManager.AddFuncRegistration(
        CONTEXT_CLIENT,
        "void",
        "SomeCoolFunc",
        "string coolArg, int sweetArg, float someOtherThing",
        "Help text for function",
        WRAPPED_MEMBER(ExampleClientFunc)
    );
}

IcepickMenu::~IcepickMenu()
{
    SDK().GetUIManager().RemoveDrawCallback("IcepickMenu");
}

void IcepickMenu::RegisterToolCommand(const CCommand& args)
{
    const char * id = args[1];
    const char * name = args[2];
    const char * tooltip = args[3];

    for (Tool & t : m_Tools)
    {
        if (t.Id.compare(id) == 0)
        {
            return; // Tool already registered
        }
    }
    m_Tools.push_back(Tool(id, name, tooltip));

    // Put tools in alphabetical order
    std::sort(m_Tools.begin(), m_Tools.end(), [](Tool & a, Tool & b)
    {
        return a.FriendlyName < b.FriendlyName;
    });
}

void IcepickMenu::RegisterToolOption(const CCommand& args)
{
    const char * toolId = args[1];
    const char * optionId = args[2];
    const char * name = args[3];
    ToolOptionType type = (ToolOptionType)std::stoi(args[4]);

    for (Tool & t : m_Tools)
    {
        if (t.Id.compare(toolId) == 0)
        {
            for (ToolOption & option : t.Options)
            {
                if (option.Id.compare(optionId) == 0)
                {
                    return; // Already registered
                }
            }

            t.Options.push_back(ToolOption(optionId, name, type));
            break;
        }
    }
}

void IcepickMenu::SetOptionDefaultValue(const CCommand& args)
{
    const char * toolId = args[1];
    const char * optionId = args[2];
    const bool isStringValue = (bool)std::stoi(args[4]);

    if (Tool * tool = GetToolFromId(toolId))
    {
        if (ToolOption * option = GetOptionFromId(tool, optionId))
        {
            if (isStringValue)
            {
                option->StringValue = args[3];
            }
            else
            {
                const float value = std::stof(args[3]);
                option->FloatValue = value;
                option->IntValue = value;
            }
        }
    }
}

void IcepickMenu::SetOptionMinMax(const CCommand& args)
{
    const char * toolId = args[1];
    const char * optionId = args[2];
    const float min = std::stof(args[3]);
    const float max = std::stof(args[4]);

    if (Tool * tool = GetToolFromId(toolId))
    {
        if (ToolOption * option = GetOptionFromId(tool, optionId))
        {
            option->Min = min;
            option->Max = max;
        }
    }
}

Tool * IcepickMenu::GetToolFromId(const char * toolId)
{
    for (Tool & t : m_Tools)
    {
        if (t.Id.compare(toolId) == 0)
        {
            return &t;
        }
    }
    return nullptr;
}

ToolOption * IcepickMenu::GetOptionFromId(Tool * tool, const char * optionId)
{
    for (ToolOption & option : tool->Options)
    {
        if (option.Id.compare(optionId) == 0)
        {
            return &option;
        }
    }
    return nullptr;
}

SQInteger IcepickMenu::ExampleClientFunc(HSQUIRRELVM v)
{
    const SQChar* str = sq_getstring.CallClient(v, 1);
    int intVal = sq_getinteger.CallClient(v, 2);
    float floatVal = sq_getfloat.CallClient(v, 3);

    spdlog::get("logger")->info("SomeCoolFunc called with {}, {}, {}", str, intVal, floatVal);

    return 0;
}


void IcepickMenu::DrawPropsGui()
{
    float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
    int NumColumns = (int)(ImGui::GetWindowContentRegionWidth() / ButtonSize);
    if (m_ModelsDisplayMode == ModelsDisplayMode::List)
    {
        NumColumns = 1;
        ButtonSize = 0;
    }

    switch (m_SpawnlistDisplayMode)
    {
    default:
    case Tree:
        if (ImGui::TreeNode("models/"))
        {
            DrawModelsDirectory(&m_ModelsList->BaseDir);
            ImGui::TreePop();
        }
        break;
    case Browser:
        ImGui::Columns(NumColumns, nullptr, false);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
        if (ImGui::Button("Root", ImVec2(ButtonSize, ButtonSize)))
        {
            m_ViewingDirectory = &m_ModelsList->BaseDir;
        }
        ImGui::PopStyleColor(3);
        ImGui::NextColumn();

        if (m_ViewingDirectory)
        {
            for (std::pair<const std::string, ModelsDirectory> & Directories : m_ViewingDirectory->SubDirectories)
            {
                ModelsDirectory & subDir = Directories.second;

                // Color directory buttons differently
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.3f, 0.6f, 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.3f, 0.7f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.3f, 0.8f, 0.4f));

                if (ImGui::Button(subDir.Path.c_str(), ImVec2(ButtonSize, ButtonSize)))
                {
                    m_ViewingDirectory = &subDir;
                }
                ImGui::PopStyleColor(3);
                ImGui::NextColumn();
            }
            DrawDirectoryModels(m_ViewingDirectory);
        }
        break;
    }
}

void IcepickMenu::DrawModelsDirectory(ModelsDirectory * dir)
{
    for (std::pair<const std::string, ModelsDirectory> & Directories : dir->SubDirectories)
    {
        ModelsDirectory & subDir = Directories.second;
        if (ImGui::TreeNode(subDir.Path.c_str()))
        {
            DrawModelsDirectory(&subDir);
            DrawDirectoryModels(&subDir);
            ImGui::TreePop();
        }
    }
}

void IcepickMenu::DrawDirectoryModels(struct ModelsDirectory * dir)
{
    switch (m_ModelsDisplayMode)
    {
    default:
    case List:
        for (int i = 0; i < dir->ModelNames.size(); ++i)
        {
            if (ImGui::Button(dir->ModelNames[i].c_str()))
            {
                DoSpawnModel(dir->Models[i]);
            }
        }
        break;
    case Grid:
        const float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
        const int NumColumns = (int)(ImGui::GetWindowContentRegionWidth() / ButtonSize);
        ImGui::Columns(NumColumns, nullptr, false);
        for (int i = 0; i < dir->ModelNames.size(); ++i)
        {
            if (ImGui::Button(dir->ModelNames[i].c_str(), ImVec2(m_SpawnmenuButtonSize, m_SpawnmenuButtonSize)))
            {
                DoSpawnModel(dir->Models[i]);
            }
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        break;
    }
}

void IcepickMenu::DrawToolsGui(float ToolsPanelWidth)
{
    for (Tool & tool : m_Tools)
    {
        if (ImGui::Button(tool.FriendlyName.c_str(), ImVec2(ToolsPanelWidth - 15, 0)))
        {
            std::string SwitchStr = "Spawnmenu_SelectTool(\"" + tool.Id + "\")";
            SDK().GetSQManager().ExecuteClientCode(SwitchStr.c_str());
            m_ViewingTool = &tool;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(tool.Description.c_str());
        }
    }
}

void IcepickMenu::DrawOptionsGui()
{
    if (m_ViewingTool == nullptr)
    {
        return;
    }

    ImGui::TextWrapped(m_ViewingTool->FriendlyName.c_str());
    ImGui::Separator();

    if (m_ViewingTool->Options.size() == 0)
    {
        ImGui::TextWrapped("No options for this tool.");
        return;
    }

    for (ToolOption & option : m_ViewingTool->Options)
    {
        switch (option.Type)
        {
        case ToolOptionType::Divider:
            ImGui::Separator();
            break;
        case ToolOptionType::Button:
            if (ImGui::Button(option.FriendlyName.c_str()))
            {
                std::string command = "Spawnmenu_UpdateToolOption( \"" + option.Id + "\", 1 )";
                SDK().GetSQManager().ExecuteClientCode(command.c_str());
                SDK().GetSQManager().ExecuteServerCode(command.c_str());
            }
            break;
        case ToolOptionType::Slider:
            if (ImGui::SliderFloat(option.FriendlyName.c_str(), &option.FloatValue, option.Min, option.Max))
            {
                std::string command = "Spawnmenu_UpdateToolOption( \"" + option.Id + "\", " + std::to_string(option.FloatValue) + " )";
                SDK().GetSQManager().ExecuteClientCode(command.c_str());
                SDK().GetSQManager().ExecuteServerCode(command.c_str());
            }
            break;
        case ToolOptionType::IntSlider:
            if (ImGui::SliderInt(option.FriendlyName.c_str(), &option.IntValue, option.Min, option.Max))
            {
                std::string command = "Spawnmenu_UpdateToolOption( \"" + option.Id + "\", " + std::to_string(option.IntValue) + " )";
                SDK().GetSQManager().ExecuteClientCode(command.c_str());
                SDK().GetSQManager().ExecuteServerCode(command.c_str());
            }
            break;
        case ToolOptionType::Text:
            ImGui::TextWrapped(option.FriendlyName.c_str());
            break;
        default:
            ImGui::Text(("Invalid tool: " + option.Id).c_str());
            break;
        }
    }
}

void IcepickMenu::DrawCategoryTab(SpawnlistTab displayTab)
{
    const float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
    const int NumColumns = (int)(ImGui::GetWindowContentRegionWidth() / ButtonSize);

    for (EntityCategory & entCategory : m_EntCategories)
    {
        if (entCategory.Tab == displayTab)
        {
            if (ImGui::CollapsingHeader(entCategory.Title.c_str()))
            {
                ImGui::Columns(NumColumns, nullptr, false);
                for (SpawnEntity & ent : entCategory.Ents)
                {
                    if (ImGui::Button(ent.FriendlyName.c_str(), ImVec2(m_SpawnmenuButtonSize, m_SpawnmenuButtonSize)))
                    {
                        std::string ExecuteString = std::string(entCategory.SpawnCode);
                        std::string Replace = "{0}";
                        size_t start_pos = ExecuteString.find(Replace);
                        if (start_pos != std::string::npos)
                        {
                            ExecuteString.replace(start_pos, Replace.length(), ent.EntityId);
                            switch (entCategory.Context)
                            {
                            case CONTEXT_CLIENT:
                                SDK().GetSQManager().ExecuteClientCode(ExecuteString.c_str());
                                break;
                            default:
                            case CONTEXT_SERVER:
                                SDK().GetSQManager().ExecuteServerCode(ExecuteString.c_str());
                                break;
                            }
                        }
                    }
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
            }
        }
    }
}

void IcepickMenu::DrawCallback()
{
    if (!m_IcepickMenuOpen)
    {
        return;
    }

    ImGui::Begin("Icepick", nullptr, ImGuiWindowFlags_MenuBar);
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::MenuItem("Props"))
            {
                m_DisplayingTab = SpawnlistTab::Props;
            }
            if (ImGui::MenuItem("Entities"))
            {
                m_DisplayingTab = SpawnlistTab::Entities;
            }
            if (ImGui::MenuItem("Weapons"))
            {
                m_DisplayingTab = SpawnlistTab::Weapons;
            }
            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::BeginMenu("Spawnlist"))
                {
                    if (ImGui::MenuItem("Tree Style", nullptr, m_SpawnlistDisplayMode == SpawnlistDisplayMode::Tree))
                    {
                        m_SpawnlistDisplayMode = SpawnlistDisplayMode::Tree;
                    }
                    if (ImGui::MenuItem("Browser Style", nullptr, m_SpawnlistDisplayMode == SpawnlistDisplayMode::Browser))
                    {
                        m_SpawnlistDisplayMode = SpawnlistDisplayMode::Browser;
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Grid", nullptr, m_ModelsDisplayMode == ModelsDisplayMode::Grid))
                    {
                        m_ModelsDisplayMode = ModelsDisplayMode::Grid;
                    }
                    if (ImGui::MenuItem("List", nullptr, m_ModelsDisplayMode == ModelsDisplayMode::List))
                    {
                        m_ModelsDisplayMode = ModelsDisplayMode::List;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Icon Size"))
                {
                    for (int size : m_SpawnmenuButtonSizes)
                    {
                        std::string Label = size > 0 ? std::to_string(size) : "Fitted";
                        if (ImGui::MenuItem(Label.c_str(), nullptr, m_SpawnmenuButtonSize == size))
                        {
                            m_SpawnmenuButtonSize = size;
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        const float ToolsWidth = std::min(ImGui::GetWindowContentRegionWidth() * 0.15f, 150.0f);
        const float OptionsWidth = std::min(ImGui::GetWindowContentRegionWidth() * 0.2f, 200.0f);
        const float SpawnlistWidth = ImGui::GetWindowContentRegionWidth() - ToolsWidth - OptionsWidth - 15;

        // Spawn Panel
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("SpawnlistPane", ImVec2(SpawnlistWidth, 0));
            switch (m_DisplayingTab)
            {
            case SpawnlistTab::Props:
                DrawPropsGui();
            case SpawnlistTab::Entities:
            case SpawnlistTab::Weapons:
                DrawCategoryTab(m_DisplayingTab);
                break;
            }
            ImGui::EndChild();
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        // Tools List
        ImGui::BeginChild("ToolsPane", ImVec2(ToolsWidth, 0), true);
        {
            DrawToolsGui(ToolsWidth);
        }
        ImGui::EndChild();
        ImGui::SameLine();

        // Tool Options
        ImGui::BeginChild("OptionsPane", ImVec2(OptionsWidth, 0), true);
        {
            DrawOptionsGui();
        }
        ImGui::EndChild();
        ImGui::SameLine();
    }
    ImGui::End();
}

void IcepickMenu::DoSpawnModel(std::string & model)
{
    std::string SpawnStr = "Spawnmenu_SpawnModel(\"" + model + "\")";
    SDK().GetSQManager().ExecuteServerCode(SpawnStr.c_str());
}

void IcepickMenu::ShowMenuCommand(const CCommand& args)
{
    m_IcepickMenuOpen = true;
    SDK().GetUIManager().SQShowCursor(0);
}

void IcepickMenu::HideMenuCommand(const CCommand& args)
{
    m_IcepickMenuOpen = false;
    SDK().GetUIManager().SQHideCursor(0);
}
