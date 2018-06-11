#include "stdafx.h"
#include <regex>
#include <algorithm>
#include "ModelsList.h"

UIManager& UIMan()
{
    return SDK().GetUIManager();
}

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&UIManager::##name), &UIManager::##name, decltype(&UIMan), &UIMan>::Call

SigScanFunc<void> d3d11DeviceFinder("materialsystem_dx11.dll", "\x48\x83\xEC\x00\x33\xC0\x89\x54\x24\x00\x4C\x8B\xC9\x48\x8B\x0D\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00", "xxx?xxxxx?xxxxxx????xxx?????");
SigScanFunc<void> d3d11ContextFinder("materialsystem_dx11.dll", "\x40\x53\x48\x83\xEC\x00\x48\x8B\x0D\x00\x00\x00\x00\x48\x8B\x01\xFF\x90\x00\x00\x00\x00\xE8\x00\x00\x00\x00", "xxxxx?xxx????xxxxx????x????");
SigScanFunc<void> d3d11SwapChainFinder("materialsystem_dx11.dll", "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x83\x3D\x00\x00\x00\x00\x00\x8B\xDA", "xxxx?xxxx?xxx?????xx");

HookedVTableFunc<decltype(&IDXGISwapChainVtbl::Present), &IDXGISwapChainVtbl::Present> IDXGISwapChain_Present;

HookedFunc<int, void*, HWND, UINT, WPARAM, LPARAM> GameWindowProc("inputsystem.dll", "\x48\x89\x54\x24\x00\x55\x56\x41\x55", "xxxx?xxxx");

HookedVTableFunc<decltype(&ISurface::VTable::LockCursor), &ISurface::VTable::LockCursor> ISurface_LockCursor;
HookedVTableFunc<decltype(&ISurface::VTable::SetCursor), &ISurface::VTable::SetCursor> ISurface_SetCursor;

UIManager::UIManager(ConCommandManager& conCommandManager, SquirrelManager& sqManager) :
    m_surface("vguimatsurface.dll", "VGUI_Surface031")
{
    m_logger = spdlog::get("logger");

    // Get pointer to d3d device
    char* funcBase = (char*)d3d11DeviceFinder.GetFuncPtr();
    int offset = *(int*)(funcBase + 16);
    m_ppD3D11Device = (ID3D11Device**)(funcBase + 20 + offset);

    // Get pointer to d3d context
    funcBase = (char*)d3d11ContextFinder.GetFuncPtr();
    offset = *(int*)(funcBase + 9);
    m_ppD3D11DeviceContext = (ID3D11DeviceContext**)(funcBase + 13 + offset);

    // Get the swap chain
    funcBase = (char*)d3d11SwapChainFinder.GetFuncPtr();
    offset = *(int*)(funcBase + 13);
    m_ppSwapChain = (IDXGISwapChain**)(funcBase + 18 + offset);

    SPDLOG_DEBUG(m_logger, "m_ppD3D11Device = {}", (void*)m_ppD3D11Device);
    SPDLOG_DEBUG(m_logger, "pD3D11Device = {}", (void*)*m_ppD3D11Device);

    SPDLOG_DEBUG(m_logger, "m_ppD3D11DeviceContext = {}", (void*)m_ppD3D11DeviceContext);
    SPDLOG_DEBUG(m_logger, "pD3D11DeviceContext = {}", (void*)*m_ppD3D11DeviceContext);

    SPDLOG_DEBUG(m_logger, "m_ppSwapChain = {}", (void*)m_ppSwapChain);
    SPDLOG_DEBUG(m_logger, "pSwapChain = {}", (void*)*m_ppSwapChain);

    InitImGui();

    IDXGISwapChain_Present.Hook((*m_ppSwapChain)->lpVtbl, WRAPPED_MEMBER(PresentHook));

    GameWindowProc.Hook(WRAPPED_MEMBER(WindowProcHook));
    ISurface_SetCursor.Hook(m_surface->m_vtable, WRAPPED_MEMBER(SetCursorHook));
    ISurface_LockCursor.Hook(m_surface->m_vtable, WRAPPED_MEMBER(LockCursorHook));

    conCommandManager.RegisterCommand("show_cursor", WRAPPED_MEMBER(ShowCursorCommand), "Set visibility of cursor", 0);

    sqManager.AddFuncRegistration(CONTEXT_CLIENT, "ShowCursor", WRAPPED_MEMBER(SQShowCursor));
    sqManager.AddFuncRegistration(CONTEXT_CLIENT, "HideCursor", WRAPPED_MEMBER(SQHideCursor));

	// Add models
	m_ModelsList = new ModelsList();
	m_ViewingDirectory = &m_ModelsList->BaseDir;

	// Add Tools
	m_Tools.push_back( Tool( "spawn_prop", "Spawner", "Spawns props when fired." ) );
	m_Tools.push_back( Tool( "remove_prop", "Remover", "Removes the prop fired at." ) );
	m_Tools.push_back( Tool( "stack_prop", "Stacker", "Stacks the same prop on itself in a direction." ) );
	m_Tools.push_back( Tool( "nudge_prop", "Nudger", "Moves props by small amounts in the opposite direction." ) );
	m_Tools.push_back( Tool( "prop_info", "Prop Info", "Prints out some debug information on a prop to the console." ) );
	m_Tools.push_back( Tool( "mirror_prop", "Timeshift Prop", "Spawns an identical prop in the other timeline if playing on Effect and Cause." ) );
	m_Tools.push_back( Tool( "camera", "Camera", "Spawns a camera which you can look through using the numpad." ) );
	m_Tools.push_back( Tool( "zipline_spawner", "Zipline", "Allows you to place ziplines in the level." ) );

	// Add Entities
	EntityCategory entsHumans = EntityCategory( "Humans", SpawnlistTab::Entities, "Spawnmenu_SpawnNpc(\"{0}\")" );
	entsHumans.Context = CONTEXT_CLIENT;
	entsHumans.Ents.push_back( SpawnEntity( "Rifle Grunt", "npc_soldier" ) );
	entsHumans.Ents.push_back( SpawnEntity( "Shotgun Grunt", "npc_soldier_shotgun" ) );
	entsHumans.Ents.push_back( SpawnEntity( "SMG Grunt", "npc_soldier_smg" ) );
	m_EntCategories.push_back( entsHumans );

	EntityCategory entsRobots = EntityCategory( "Robots", SpawnlistTab::Entities, "Spawnmenu_SpawnNpc(\"{0}\")" );
	entsRobots.Context = CONTEXT_CLIENT;
	entsRobots.Ents.push_back( SpawnEntity( "Spectre", "npc_spectre" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Stalker", "npc_stalker" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Zombie Stalker", "npc_stalker_zombie" ) ); 
	entsRobots.Ents.push_back( SpawnEntity( "Zombie Stalker (Mossy)", "npc_stalker_zombie_mossy" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Reaper", "npc_super_spectre" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Drone", "npc_drone" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Rocket Drone", "npc_drone_rocket" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Plasma Drone", "npc_drone_plasma" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Worker Drone", "npc_drone_worker" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Tick", "npc_frag_drone" ) );
	entsRobots.Ents.push_back( SpawnEntity( "Marvin", "npc_marvin" ) );
	m_EntCategories.push_back( entsRobots );

	EntityCategory entsTitans = EntityCategory( "Titans", SpawnlistTab::Entities, "Spawnmenu_SpawnNpc(\"{0}\")" );
	entsTitans.Context = CONTEXT_CLIENT;
	entsTitans.Ents.push_back( SpawnEntity( "BT-7274", "npc_titan_bt" ) );
	entsTitans.Ents.push_back( SpawnEntity( "BT-7274 2", "npc_titan_bt_spare" ) );
	entsTitans.Ents.push_back( SpawnEntity( "Atlas", "npc_titan_atlas" ) );
	entsTitans.Ents.push_back( SpawnEntity( "Stryder", "npc_titan_stryder" ) );
	entsTitans.Ents.push_back( SpawnEntity( "Ogre", "npc_titan_ogre" ) );
	m_EntCategories.push_back( entsTitans );

	// Add Weapons
	EntityCategory tools = EntityCategory( "Tools", SpawnlistTab::Weapons, "Spawnmenu_GiveWeapon(\"{0}\")" );
	tools.Ents.push_back( SpawnEntity( "Toolgun", "mp_weapon_shotgun_pistol" ) );
	m_EntCategories.push_back( tools );

	EntityCategory pilotPrimaries = EntityCategory( "Primaries - Pilot", SpawnlistTab::Weapons, "Spawnmenu_GiveWeapon(\"{0}\")" );
	pilotPrimaries.Ents.push_back( SpawnEntity( "R-201 Carbine", "mp_weapon_rspn101" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "R-101 Carbine", "mp_weapon_rspn101_og" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Hemlok BF-R", "mp_weapon_hemlok" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "G2A5", "mp_weapon_g2" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "V-47 Flatline", "mp_weapon_vinson" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "CAR", "mp_weapon_car" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Alternator", "mp_weapon_alternator_smg" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Volt", "mp_weapon_hemlok_smg" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "R-47", "mp_weapon_r97" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Spitfire", "mp_weapon_lmg" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "L_STAR", "mp_weapon_lstar" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "X-55 Devotion", "mp_weapon_esaw" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Kraber-AP Sniper", "mp_weapon_sniper" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "D-2 Double Take", "mp_weapon_doubletake" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Longbow-DMR", "mp_weapon_dmr" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "EVA-8 Auto", "mp_weapon_shotgun" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Mastiff", "mp_weapon_mastiff" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Sidewinder SMR", "mp_weapon_smr" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "EPG-1", "mp_weapon_epg" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "R-6P Softball", "mp_weapon_softball" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "EM-4 Cold War", "mp_weapon_pulse_lmg" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "Wingman Elite", "mp_weapon_wingman_n" ) );
	pilotPrimaries.Ents.push_back( SpawnEntity( "SA-3 Mozambique", "mp_weapon_shotgun_pistol" ) );
	m_EntCategories.push_back( pilotPrimaries );

	EntityCategory pilotSecondaries = EntityCategory( "Secondaries - Pilot", SpawnlistTab::Weapons, "Spawnmenu_GiveWeapon(\"{0}\")" );
	pilotSecondaries.Ents.push_back( SpawnEntity( "Hammond P2016", "mp_weapon_semipistol" ) );
	pilotSecondaries.Ents.push_back( SpawnEntity( "RE-45 Auto", "mp_weapon_autopistol" ) );
	pilotSecondaries.Ents.push_back( SpawnEntity( "B3 Wingman", "mp_weapon_wingman" ) );
	pilotSecondaries.Ents.push_back( SpawnEntity( "Charge Rifle", "mp_weapon_defender" ) );
	pilotSecondaries.Ents.push_back( SpawnEntity( "MGL Mag Launcher", "mp_weapon_mgl" ) );
	pilotSecondaries.Ents.push_back( SpawnEntity( "LG-97 Thunderbolt", "mp_weapon_arc_launcher" ) );
	pilotSecondaries.Ents.push_back( SpawnEntity( "Archer", "mp_weapon_rocket_launcher" ) );
	m_EntCategories.push_back( pilotSecondaries );

	EntityCategory pilotAbilities = EntityCategory( "Pilot Abilities", SpawnlistTab::Weapons, "Spawnmenu_GiveAbility(\"{0}\")" );
	pilotAbilities.Ents.push_back( SpawnEntity( "Cloak", "mp_ability_cloak" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Grapple", "mp_ability_grapple" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Stim", "mp_ability_heal" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Holopilot", "mp_ability_holopilot" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Phase Shift", "mp_ability_shifter" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Pulse Blade", "mp_weapon_grenade_sonar" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "A-Wall", "mp_weapon_deployable_cover" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Timeshift [Effect and Cause]", "mp_ability_timeshift" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Arcblast [Dev]", "mp_ability_arc_blast" ) );
	pilotAbilities.Ents.push_back( SpawnEntity( "Super Phase Shift [Dev]", "mp_ability_shifter_super" ) );
	m_EntCategories.push_back( pilotAbilities );

	EntityCategory pilotGrenades = EntityCategory( "Pilot Ordnance", SpawnlistTab::Weapons, "Spawnmenu_GiveGrenade(\"{0}\")" );
	pilotGrenades.Ents.push_back( SpawnEntity( "Frag Grenade", "mp_weapon_frag_grenade" ) );
	pilotGrenades.Ents.push_back( SpawnEntity( "Firestar", "mp_weapon_thermite_grenade" ) );
	pilotGrenades.Ents.push_back( SpawnEntity( "Electric Smoke Grenade", "mp_weapon_grenade_electric_smoke" ) );
	pilotGrenades.Ents.push_back( SpawnEntity( "Arc Grenade", "mp_weapon_grenade_emp" ) );
	pilotGrenades.Ents.push_back( SpawnEntity( "Gravity Star", "mp_weapon_grenade_gravity" ) );
	pilotGrenades.Ents.push_back( SpawnEntity( "Satchel", "mp_weapon_satchel" ) );
	m_EntCategories.push_back( pilotGrenades );

	EntityCategory pilotMelee = EntityCategory( "Pilot Melee", SpawnlistTab::Weapons, "Spawnmenu_GiveMelee(\"{0}\")" );
	pilotMelee.Ents.push_back( SpawnEntity( "Standard Melee", "melee_pilot_emptyhanded" ) );
	pilotMelee.Ents.push_back( SpawnEntity( "Sword Melee", "melee_pilot_sword" ) );
	m_EntCategories.push_back( pilotMelee );
}

UIManager::~UIManager()
{
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
}

void UIManager::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // TODO: Make this a bit more reliable
    HWND wnd = FindWindow(NULL, L"Titanfall 2");
    m_logger->info("Game window = {}", (void*)wnd);

    ImGui_ImplDX11_Init(wnd, *m_ppD3D11Device, *m_ppD3D11DeviceContext);
    ImGui::StyleColorsDark();
    ImGui_ImplDX11_CreateDeviceObjects();
}

void UIManager::ShowCursorCommand(const CCommand& args)
{
    if (strcmp(args[1], "1") == 0)
    {
        SQShowCursor(0); // TODO: Fix this
    }
    else if (strcmp(args[1], "0") == 0)
    {
        SQHideCursor(0); // TODO: Fix this
    }
    else
    {
        m_logger->error("Invalid argument to show_cursor, must be 1 or 0");
    }
}

SQInteger UIManager::SQShowCursor(HSQUIRRELVM v)
{
    m_enableCursor = true;
    m_logger->info("Showing cursor");
    m_surface->m_vtable->UnlockCursor(m_surface);
    m_surface->m_vtable->SetCursor(m_surface, dc_arrow);
    return 0;
}

SQInteger UIManager::SQHideCursor(HSQUIRRELVM v)
{
    m_enableCursor = false;
    m_logger->info("Hiding cursor");
    return 0;
}

void UIManager::DrawGUI()
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    if (corner != -1)
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
    if (ImGui::Begin("DebugOverlay", nullptr, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
        ImGui::Text("m_enableCursor: %d", (int)m_enableCursor);
        ImGui::Text("IsCursorVisible: %d", m_surface->m_vtable->IsCursorVisible(m_surface));
        ImGui::Text("WantCaptureMouse: %d", ImGui::GetIO().WantCaptureMouse);
        ImGui::Text("WantCaptureKeyboard: %d", ImGui::GetIO().WantCaptureKeyboard);
        ImGui::End();
    }
}

void UIManager::AddDrawCallback(const std::string& name, const std::function<void()>& func)
{
    m_drawCallbacks[name] = func;
}

void UIManager::RemoveDrawCallback(const std::string& name)
{
    m_drawCallbacks.erase(name);
}

bool IsKeyMsg(UINT uMsg)
{
    return uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST;
}

bool IsMouseMsg(UINT uMsg)
{
    return uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST;
}

bool UIManager::IsACursorVisible()
{
    return m_enableCursor || m_surface->m_vtable->IsCursorVisible(m_surface);
}

void UIManager::DrawPropsGui()
{
	float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
	int NumColumns = (int) (ImGui::GetWindowContentRegionWidth() / ButtonSize);
	if( m_ModelsDisplayMode == ModelsDisplayMode::List )
	{
		NumColumns = 1;
		ButtonSize = 0;
	}

	switch( m_SpawnlistDisplayMode )
	{
		default:
		case Tree:
			if( ImGui::TreeNode( "models/" ) )
			{
				DrawModelsDirectory( &m_ModelsList->BaseDir );
				ImGui::TreePop();
			}
			break;
		case Browser:
			ImGui::Columns( NumColumns, nullptr, false );
			ImGui::PushStyleColor( ImGuiCol_Button, (ImVec4) ImColor::HSV( 0.0f, 0.6f, 0.6f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV( 0.0f, 0.7f, 0.7f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV( 0.0f, 0.8f, 0.8f ) );
			if( ImGui::Button( "Root", ImVec2( ButtonSize, ButtonSize ) ) )
			{
				m_ViewingDirectory = &m_ModelsList->BaseDir;
			}
			ImGui::PopStyleColor( 3 );
			ImGui::NextColumn();

			if( m_ViewingDirectory )
			{
				for( std::pair<const std::string, ModelsDirectory> & Directories : m_ViewingDirectory->SubDirectories )
				{
					ModelsDirectory & subDir = Directories.second;

					// Color directory buttons differently
					ImGui::PushStyleColor( ImGuiCol_Button, (ImVec4) ImColor::HSV( 0.3f, 0.6f, 0.3f ) );
					ImGui::PushStyleColor( ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV( 0.3f, 0.7f, 0.4f ) );
					ImGui::PushStyleColor( ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV( 0.3f, 0.8f, 0.4f ) );

					if( ImGui::Button( subDir.Path.c_str(), ImVec2( ButtonSize, ButtonSize ) ) )
					{
						m_ViewingDirectory = &subDir;
					}
					ImGui::PopStyleColor( 3 );
					ImGui::NextColumn();
				}
				DrawDirectoryModels( m_ViewingDirectory );
			}
			break;
	}
}

void UIManager::DrawModelsDirectory( ModelsDirectory * dir )
{
	for( std::pair<const std::string, ModelsDirectory> & Directories : dir->SubDirectories )
	{
		ModelsDirectory & subDir = Directories.second;
		if( ImGui::TreeNode( subDir.Path.c_str() ) )
		{
			DrawModelsDirectory( &subDir );
			DrawDirectoryModels( &subDir );
			ImGui::TreePop();
		}
	}
}

void UIManager::DrawDirectoryModels( struct ModelsDirectory * dir )
{
	switch( m_ModelsDisplayMode )
	{
		default:
		case List:
			for( int i = 0; i < dir->ModelNames.size(); ++i )
			{
				if( ImGui::Button( dir->ModelNames[i].c_str() ) )
				{
					DoSpawnModel( dir->Models[i] );
				}
			}
			break;
		case Grid:
			const float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
			const int NumColumns = (int) (ImGui::GetWindowContentRegionWidth() / ButtonSize);
			ImGui::Columns( NumColumns, nullptr, false );
			for( int i = 0; i < dir->ModelNames.size(); ++i )
			{
				if( ImGui::Button( dir->ModelNames[i].c_str(), ImVec2( m_SpawnmenuButtonSize, m_SpawnmenuButtonSize ) ) )
				{
					DoSpawnModel( dir->Models[i] );
				}
				ImGui::NextColumn();
			}
			ImGui::Columns( 1 );
			break;
	}
}

void UIManager::DrawToolsGui( float ToolsPanelWidth )
{
	for( Tool & tool : m_Tools )
	{
		if( ImGui::Button( tool.FriendlyName.c_str(), ImVec2( ToolsPanelWidth - 15, 0 ) ) )
		{
			std::string SwitchStr = "Spawnmenu_SelectTool(\"" + tool.Id + "\")";
			SDK().GetSQManager().ExecuteClientCode( SwitchStr.c_str() );
		}
		if( ImGui::IsItemHovered() )
		{
			ImGui::SetTooltip( tool.Description.c_str() );
		}
	}
}

void UIManager::DrawCategoryTab( SpawnlistTab displayTab )
{
	const float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
	const int NumColumns = (int) (ImGui::GetWindowContentRegionWidth() / ButtonSize);

	for( EntityCategory & entCategory : m_EntCategories )
	{
		if( entCategory.Tab == displayTab )
		{
			if( ImGui::CollapsingHeader( entCategory.Title.c_str() ) )
			{
				ImGui::Columns( NumColumns, nullptr, false );
				for ( SpawnEntity & ent : entCategory.Ents )
				{
					if( ImGui::Button( ent.FriendlyName.c_str(), ImVec2( m_SpawnmenuButtonSize, m_SpawnmenuButtonSize ) ) )
					{
						std::string ExecuteString = std::string( entCategory.SpawnCode );
						std::string Replace = "{0}";
						size_t start_pos = ExecuteString.find( Replace );
						if( start_pos != std::string::npos )
						{
							ExecuteString.replace( start_pos, Replace.length(), ent.EntityId );
							switch( entCategory.Context )
							{
								case CONTEXT_CLIENT:
									SDK().GetSQManager().ExecuteClientCode( ExecuteString.c_str() );
									break;
								default:
								case CONTEXT_SERVER:
									SDK().GetSQManager().ExecuteServerCode( ExecuteString.c_str() );
									break;
							}
						}
					}
					ImGui::NextColumn();
				}
				ImGui::Columns( 1 );
			}
		}
	}
}

void UIManager::DoSpawnModel( std::string & model )
{
	if( true )
	{
		std::string SpawnStr = "Spawnmenu_SpawnModel_Broke(\"" + model + "\")";
		SDK().GetSQManager().ExecuteServerCode( SpawnStr.c_str() );
	}
	else
	{
		std::string SpawnStr = "Spawnmenu_SpawnModel(\"" + model + "\")";
		SDK().GetSQManager().ExecuteClientCode( SpawnStr.c_str() );
	}
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int UIManager::WindowProcHook(void* game, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Don't pass to imgui if there's no cursor visible
    if (uMsg == WM_SETCURSOR || !IsACursorVisible())
    {
        return GameWindowProc(game, hWnd, uMsg, wParam, lParam);
    }

    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    
    // Do not pass to game if we're forcing the cursor
    bool forcedCursor = m_enableCursor && !m_surface->m_vtable->IsCursorVisible(m_surface);

    // Only block from game if imgui capturing
    if (IsMouseMsg(uMsg) && (forcedCursor || ImGui::GetIO().WantCaptureMouse))
    {
        return 0;
    }
    
    if (IsKeyMsg(uMsg) && (forcedCursor || ImGui::GetIO().WantCaptureKeyboard))
    {
        return 0;
    }

    return GameWindowProc(game, hWnd, uMsg, wParam, lParam);
}

static bool ImGui_UpdateMouseCursor(ISurface* surface)
{
    ImGuiIO& io = ImGui::GetIO();
    //if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
    //    return false;

    ImGuiMouseCursor imgui_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        ISurface_SetCursor(surface, dc_none);
    }
    else
    {
        // Hardware cursor type
        unsigned int cursor = dc_arrow;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        cursor = dc_arrow; break;
        case ImGuiMouseCursor_TextInput:    cursor = dc_ibeam; break;
        case ImGuiMouseCursor_ResizeAll:    cursor = dc_sizeall; break;
        case ImGuiMouseCursor_ResizeEW:     cursor = dc_sizewe; break;
        case ImGuiMouseCursor_ResizeNS:     cursor = dc_sizens; break;
        case ImGuiMouseCursor_ResizeNESW:   cursor = dc_sizenesw; break;
        case ImGuiMouseCursor_ResizeNWSE:   cursor = dc_sizenwse; break;
        }
        ISurface_SetCursor(surface, cursor);
    }
    return true;
}

void UIManager::SetCursorHook(ISurface* surface, unsigned int cursor)
{
    // If no cursors, let the engine deal with it
    if (!IsACursorVisible())
    {
        return ISurface_SetCursor(surface, cursor);
    }

    // If there's a cursor, and ImGui is capturing, let that handle it
    if (ImGui::GetIO().WantCaptureMouse)
    {
        ImGui_UpdateMouseCursor(surface);
        return;
    }
    
    // Otherwise let the game handle it
    ISurface_SetCursor(surface, cursor);
}

void UIManager::LockCursorHook(ISurface* surface)
{
    if (!m_enableCursor || m_surface->m_vtable->IsCursorVisible(m_surface))
    {
        return ISurface_LockCursor(surface);
    }
}

HRESULT UIManager::PresentHook(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)
{
    ImGui_ImplDX11_NewFrame();
    //DrawGUI();
    for (const auto& entry : m_drawCallbacks)
    {
        entry.second();
    }

	if( m_enableCursor )
	{
		ImGui::Begin( "Icepick", nullptr, ImGuiWindowFlags_MenuBar );
		{
			if( ImGui::BeginMenuBar() )
			{
				if( ImGui::MenuItem( "Props" ) )
				{
					m_DisplayingTab = SpawnlistTab::Props;
				}
				if( ImGui::MenuItem( "Entities" ) )
				{
					m_DisplayingTab = SpawnlistTab::Entities;
				}
				if( ImGui::MenuItem( "Weapons" ) )
				{
					m_DisplayingTab = SpawnlistTab::Weapons;
				}
				if( ImGui::BeginMenu( "Options" ) )
				{
					if( ImGui::BeginMenu( "Spawnlist" ) )
					{
						if( ImGui::MenuItem( "Tree Style", nullptr, m_SpawnlistDisplayMode == SpawnlistDisplayMode::Tree ) )
						{
							m_SpawnlistDisplayMode = SpawnlistDisplayMode::Tree;
						}
						if( ImGui::MenuItem( "Browser Style", nullptr, m_SpawnlistDisplayMode == SpawnlistDisplayMode::Browser ) )
						{
							m_SpawnlistDisplayMode = SpawnlistDisplayMode::Browser;
						}
						ImGui::Separator();
						if( ImGui::MenuItem( "Grid", nullptr, m_ModelsDisplayMode == ModelsDisplayMode::Grid ) )
						{
							m_ModelsDisplayMode = ModelsDisplayMode::Grid;
						}
						if( ImGui::MenuItem( "List", nullptr, m_ModelsDisplayMode == ModelsDisplayMode::List ) )
						{
							m_ModelsDisplayMode = ModelsDisplayMode::List;
						}
						ImGui::EndMenu();
					}
					if( ImGui::BeginMenu( "Icon Size" ) )
					{
						for( int size : m_SpawnmenuButtonSizes )
						{
							std::string Label = size > 0 ? std::to_string( size ) : "Fitted";
							if( ImGui::MenuItem( Label.c_str(), nullptr, m_SpawnmenuButtonSize == size ) )
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

			const float ToolsWidth = std::min( ImGui::GetWindowContentRegionWidth() * 0.15f, 150.0f );
			const float OptionsWidth = std::min( ImGui::GetWindowContentRegionWidth() * 0.2f, 200.0f );
			const float SpawnlistWidth = ImGui::GetWindowContentRegionWidth() - ToolsWidth - OptionsWidth - 15;

			// Spawn Panel
			ImGui::BeginGroup();
			{
				ImGui::BeginChild( "SpawnlistPane", ImVec2( SpawnlistWidth, 0 ) );
				switch( m_DisplayingTab )
				{
					case SpawnlistTab::Props:
						DrawPropsGui();
					case SpawnlistTab::Entities:
					case SpawnlistTab::Weapons:
						DrawCategoryTab( m_DisplayingTab );
						break;
					default:
						DrawGUI();
						break;
				}
				ImGui::EndChild();
			}
			ImGui::EndGroup();
			ImGui::SameLine();

			// Tools List
			ImGui::BeginChild( "ToolsPane", ImVec2( ToolsWidth, 0 ), true );
			{
				DrawToolsGui( ToolsWidth );
			}
			ImGui::EndChild();
			ImGui::SameLine();

			// Tool Options
			ImGui::BeginChild( "OptionsPane", ImVec2( OptionsWidth, 0 ), true );
			{
				ImGui::Text( "Options panel" );
			}
			ImGui::EndChild();
			ImGui::SameLine();
		}
		ImGui::End();
	}
	
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return IDXGISwapChain_Present(SwapChain, SyncInterval, Flags);
}
