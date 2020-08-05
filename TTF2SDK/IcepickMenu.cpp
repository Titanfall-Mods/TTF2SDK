#include "stdafx.h"
#include "ModelsList.h"
#include <iosfwd>

IcepickMenu& Menu()
{
    return SDK().GetIcepickMenu();
}

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&IcepickMenu::##name), &IcepickMenu::##name, decltype(&Menu), &Menu>::Call

IcepickMenu::IcepickMenu(ConCommandManager& conCommandManager, UIManager& uiManager, SquirrelManager& sqManager, FileSystemManager & fsManager )
{
    m_logger = spdlog::get("logger");

    // Commands
    conCommandManager.RegisterCommand("show_icepick_menu", WRAPPED_MEMBER(ShowMenuCommand), "Shows the Icepick Menu", 0);
    conCommandManager.RegisterCommand("hide_icepick_menu", WRAPPED_MEMBER(HideMenuCommand), "Hides the Icepick Menu", 0);

	conCommandManager.RegisterCommand( "toggle_toolgun", WRAPPED_MEMBER( ToggleToolgunCommand ), "Toggle the toolgun", 0 );
	conCommandManager.RegisterCommand( "undo", WRAPPED_MEMBER( DummyConCommand ), "Dummy function for scripted undo functionality", 0 );
	conCommandManager.RegisterCommand( "instant_respawn", WRAPPED_MEMBER( DummyConCommand ), "Dummy function for scripted instant respawn", 0 );

	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "int", "IsSpawnMenuOpen", "", "Help text", WRAPPED_MEMBER( IsMenuShowing ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "int", "IsInvincibilityEnabled", "", "Help text", WRAPPED_MEMBER( IsInvincibilityEnabled ) );
	
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "EnableEditMode", "", "Help text", WRAPPED_MEMBER( EnableEditMode ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "DisableEditMode", "", "Help text", WRAPPED_MEMBER( DisableEditMode ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "int", "IsEditModeEnabled", "", "Help text", WRAPPED_MEMBER( IsEditModeEnabled ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "int", "IsEditModeEnabled", "", "Help text", WRAPPED_MEMBER( IsEditModeEnabled ) );

	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "ClearTools", "", "Help text", WRAPPED_MEMBER( ClearTools ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "RegisterTool", "string id, string friendlyName, string tooltip", "Help text", WRAPPED_MEMBER( RegisterTool ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "AddDividerOption", "string toolId", "Help text", WRAPPED_MEMBER( AddToolOption_Divider ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "AddTextOption", "string toolId, string text", "Help text", WRAPPED_MEMBER( AddToolOption_Text ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "AddButtonOption", "string toolId, string id, string buttonText", "Help text", WRAPPED_MEMBER( AddToolOption_Button ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "AddSliderOption", "string toolId, string id, string name, float default, float min, float max", "Help text", WRAPPED_MEMBER( AddToolOption_Slider ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "AddIntSliderOption", "string toolId, string id, string name, int default, int min, int max", "Help text", WRAPPED_MEMBER( AddToolOption_IntSlider ) );

	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "ClearSaveBuffer", "", "Help text", WRAPPED_MEMBER( ClearSaveBuffer ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "AddSaveItem", "string data", "Help text", WRAPPED_MEMBER( AddSaveItem ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "WriteSaveBufferToFile", "string fileName", "Help text", WRAPPED_MEMBER( WriteSaveBufferToFile ) );

	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "array", "GetSaveFiles", "", "Help text", WRAPPED_MEMBER( GetSaveFiles ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "string", "LoadSaveFileContents", "string fileName", "Help text", WRAPPED_MEMBER( LoadSaveFileContents ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "ShowLoadingModal", "", "Help text", WRAPPED_MEMBER( ShowLoadingModal ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "HideLoadingModal", "", "Help text", WRAPPED_MEMBER( HideLoadingModal ) );

	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "ClearSpawnmenu", "", "Help text", WRAPPED_MEMBER( ClearSpawnmenu ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "RegisterSpawnmenuPage", "string id, string friendlyName", "Help text", WRAPPED_MEMBER( RegisterSpawnmenuPage ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "RegisterPageCategory", "string pageId, string id, string friendlyName, string callbackName", "Help text", WRAPPED_MEMBER( RegisterPageCategory ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "RegisterCategoryItem", "string categoryId, string itemId, string friendlyName", "Help text", WRAPPED_MEMBER( RegisterCategoryItem ) );

    // Add models
    m_ModelsList = new ModelsList( fsManager );
    m_ViewingDirectory = &m_ModelsList->BaseDir;

    uiManager.AddDrawCallback("IcepickMenu", std::bind(&IcepickMenu::DrawCallback, this));
}

IcepickMenu::~IcepickMenu()
{
    SDK().GetUIManager().RemoveDrawCallback("IcepickMenu");
}

SQInteger IcepickMenu::RegisterTool( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallServer( v, 1 );
	const SQChar * name = sq_getstring.CallServer( v, 2 );
	const SQChar * tooltip = sq_getstring.CallServer( v, 3 );

	for( Tool & t : m_Tools )
	{
		if( t.Id.compare( toolId ) == 0 )
		{
			return 0; // Tool already registered
		}
	}
	m_Tools.push_back( Tool( toolId, name, tooltip ) );

	// Put tools in alphabetical order
	std::sort( m_Tools.begin(), m_Tools.end(), []( Tool & a, Tool & b )
	{
		return a.FriendlyName < b.FriendlyName;
	} );

	return 0;
}

SQInteger IcepickMenu::AddToolOption_Divider( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallServer( v, 1 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		tool->Options.push_back( ToolOption( toolId, "", ToolOptionType::Divider ) );
	}
	return 0;
}

SQInteger IcepickMenu::AddToolOption_Text( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallServer( v, 1 );
	const SQChar * text = sq_getstring.CallServer( v, 2 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		tool->Options.push_back( ToolOption( text, text, ToolOptionType::Text ) );
	}
	return 0;
}

SQInteger IcepickMenu::AddToolOption_Button( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallServer( v, 1 );
	const SQChar * optionId = sq_getstring.CallServer( v, 2 );
	const SQChar * text = sq_getstring.CallServer( v, 3 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		tool->Options.push_back( ToolOption( optionId, text, ToolOptionType::Button ) );
	}
	return 0;
}

SQInteger IcepickMenu::AddToolOption_Slider( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallServer( v, 1 );
	const SQChar * optionId = sq_getstring.CallServer( v, 2 );
	const SQChar * name = sq_getstring.CallServer( v, 3 );
	float defaultValue = sq_getfloat.CallServer( v, 4 );
	float min = sq_getfloat.CallServer( v, 5 );
	float max = sq_getfloat.CallServer( v, 6 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		ToolOption option = ToolOption( optionId, name, ToolOptionType::Slider );
		option.FloatValue = defaultValue;
		option.IntValue = (int) defaultValue;
		option.Min = min;
		option.Max = max;
		tool->Options.push_back( option );
	}
	return 0;
}

SQInteger IcepickMenu::AddToolOption_IntSlider( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallServer( v, 1 );
	const SQChar * optionId = sq_getstring.CallServer( v, 2 );
	const SQChar * name = sq_getstring.CallServer( v, 3 );
	int defaultValue = sq_getinteger.CallServer( v, 4 );
	int min = sq_getinteger.CallServer( v, 5 );
	int max = sq_getinteger.CallServer( v, 6 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		ToolOption option = ToolOption( optionId, name, ToolOptionType::IntSlider );
		option.FloatValue = (float) defaultValue;
		option.IntValue = defaultValue;
		option.Min = min;
		option.Max = max;
		tool->Options.push_back( option );
	}
	return 0;
}

SQInteger IcepickMenu::ClearSpawnmenu( HSQUIRRELVM v )
{
	m_Pages.clear();
	return 0;
}

SQInteger IcepickMenu::RegisterSpawnmenuPage( HSQUIRRELVM v )
{
	const SQChar * pageId = sq_getstring.CallClient( v, 1 );
	const SQChar * pageName = sq_getstring.CallClient( v, 2 );

	SpawnmenuPage newPage = SpawnmenuPage( pageId, pageName );
	m_Pages.push_back( newPage );

	return 0;
}

SQInteger IcepickMenu::RegisterPageCategory( HSQUIRRELVM v )
{
	const SQChar * pageId = sq_getstring.CallClient( v, 1 );
	const SQChar * categoryId = sq_getstring.CallClient( v, 2 );
	const SQChar * categoryName = sq_getstring.CallClient( v, 3 );
	const SQChar * callbackName = sq_getstring.CallClient( v, 4 );

	if( SpawnmenuPage * page = GetPageFromId( pageId ) )
	{
		EntityCategory newCategory = EntityCategory( categoryId, categoryName, callbackName );
		page->Categories.push_back( newCategory );
	}
	else
	{
		m_logger->error( "Could not register category! No page found with id {}", pageId );
	}

	return 0;
}

SQInteger IcepickMenu::RegisterCategoryItem( HSQUIRRELVM v )
{
	const SQChar * categoryId = sq_getstring.CallClient( v, 1 );
	const SQChar * itemId = sq_getstring.CallClient( v, 2 );
	const SQChar * itemName = sq_getstring.CallClient( v, 3 );

	if( EntityCategory * category = GetCategoryFromId( categoryId ) )
	{
		SpawnEntity newEnt = SpawnEntity( itemName, itemId );
		category->Ents.push_back( newEnt );
	}
	else
	{
		m_logger->error( "Could not register item! No category found with id {}", categoryId );
	}

	return 0;
}

SQInteger IcepickMenu::ClearSaveBuffer( HSQUIRRELVM v )
{
	m_SaveBuffer.clear();
	return 0;
}

SQInteger IcepickMenu::AddSaveItem( HSQUIRRELVM v )
{
	const SQChar * itemData = sq_getstring.CallServer( v, 1 );
	m_SaveBuffer.push_back( std::string( itemData ) );
	return 0;
}

SQInteger IcepickMenu::WriteSaveBufferToFile( HSQUIRRELVM v )
{
	const SQChar * fileName = sq_getstring.CallClient( v, 1 );

	std::string filePath = ( SDK().GetFSManager().GetSavesPath() / fileName ).string();

	std::ofstream saveFile;
	saveFile.open( filePath );

	m_logger->info( "Saving to {}", filePath );
	for( std::string & entry : m_SaveBuffer )
	{
		m_logger->info( "{}", entry );
		saveFile << entry << "\n";
	}
	saveFile.close();

	return 0;
}

SQInteger IcepickMenu::GetSaveFiles( HSQUIRRELVM v )
{
	sq_newarray.CallClient( v, 0 );
	for( auto & dirIter : fs::recursive_directory_iterator( SDK().GetFSManager().GetSavesPath() ) )
	{
		if( dirIter.status().type() == fs::file_type::directory )
		{
			continue;
		}

		fs::path path = dirIter.path();
		std::string pathString = path.string();
		sq_pushstring.CallClient( v, pathString.c_str(), -1 );
		sq_arrayappend.CallClient( v, -2 );
	}

	return 1;
}

SQInteger IcepickMenu::LoadSaveFileContents( HSQUIRRELVM v )
{
	const SQChar * fileName = sq_getstring.CallServer( v, 1 );
	std::string filePath = ( SDK().GetFSManager().GetSavesPath() / fileName ).string();

	std::ifstream saveFile( filePath );
	std::string saveData = Util::ReadFileToString( saveFile );
	sq_pushstring.CallServer( v, saveData.c_str(), -1 );

	return 1;
}

SQInteger IcepickMenu::ShowLoadingModal( HSQUIRRELVM v )
{
	m_ShowLoadingModal = true;
	m_logger->info( "Showing loading modal" );
	return 0;
}

SQInteger IcepickMenu::HideLoadingModal( HSQUIRRELVM v )
{
	m_ShowLoadingModal = false;
	m_logger->info( "Hiding loading modal" );
	return 0;
}

SQInteger IcepickMenu::EnableEditMode( HSQUIRRELVM v )
{
	m_EditModeEnabled = true;
	m_logger->info( "Enabled edit mode" );
	return 0;
}

SQInteger IcepickMenu::DisableEditMode( HSQUIRRELVM v )
{
	m_EditModeEnabled = false;
	m_logger->info( "Disabled edit mode" );
	return 0;
}

SQInteger IcepickMenu::IsEditModeEnabled( HSQUIRRELVM v )
{
	sq_pushinteger.CallServer( v, (int) m_EditModeEnabled );
	return 1;
}

SQInteger IcepickMenu::IsInvincibilityEnabled( HSQUIRRELVM v )
{
	sq_pushinteger.CallServer( v, (int) m_IsInvincibilityEnabled );
	return 1;
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

SpawnmenuPage * IcepickMenu::GetPageFromId( const char * pageId )
{
	for( SpawnmenuPage & page : m_Pages )
	{
		if( page.Id.compare( pageId ) == 0 )
		{
			return &page;
		}
	}
	return nullptr;
}

EntityCategory * IcepickMenu::GetCategoryFromId( const char * categoryId )
{
	for( SpawnmenuPage & page : m_Pages )
	{
		for( EntityCategory & category : page.Categories )
		{
			if( category.Id.compare( categoryId ) == 0 )
			{
				return &category;
			}
		}
	}
	return nullptr;
}

void IcepickMenu::ClearSearch()
{
	for( int i = 0; i < SearchMaxChars; ++i )
	{
		m_SearchInput[i] = '\0';
	}
}

void IcepickMenu::UpdateSearchResults()
{
	int searchInputLength = strlen( m_SearchInput );
	if( m_CachedSearchInputLength != searchInputLength )
	{
		std::vector<std::string *> tempResults;

		m_SearchResults.clear();
		if( m_ModelsList->CurrentSpawnlist != nullptr )
		{
			for( std::string & model : m_ModelsList->CurrentSpawnlist->Props )
			{
				m_SearchResults.push_back( &model );
			}
		}

		std::vector<std::string> searchTokens = Util::Split( m_SearchInput, ' ' );
		for( std::string searchToken : searchTokens )
		{
			// Find all results that match the current token
			tempResults.clear();
			for( std::string * model : m_SearchResults )
			{
				if( model->find( searchToken ) != std::string::npos )
				{
					tempResults.push_back( model );
				}
			}

			// Save the current results for the next token
			m_SearchResults.clear();
			for( std::string * model : tempResults )
			{
				m_SearchResults.push_back( model );
			}
		}

		m_CachedSearchInputLength = searchInputLength;
	}
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

	int searchInputLength = strlen( m_SearchInput );
	ImGui::InputText( "Search", m_SearchInput, IM_ARRAYSIZE( m_SearchInput ) );
	if( searchInputLength > 0 )
	{
		ImGui::SameLine();
		if( ImGui::Button( "Clear Search" ) )
		{
			ClearSearch();
		}
	}

	if( searchInputLength > 0 )
	{
		UpdateSearchResults();
		DrawSearchResults();
	}
	else
	{
		switch( m_SpawnlistDisplayMode )
		{
			default:
			case Tree:
				ImGui::SetNextTreeNodeOpen( true, ImGuiCond_FirstUseEver );
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
}

void IcepickMenu::DrawSearchResults()
{
	std::string searchText = "Searching for \"replaceme\"";
	searchText = std::regex_replace( searchText, std::regex( "replaceme" ), m_SearchInput );
	ImGui::Text( searchText.c_str() );

	if( m_SearchResults.size() > 0 )
	{
		for( std::string * model : m_SearchResults )
		{
			if( ImGui::Button( model->c_str() ) )
			{
				DoSpawnModel( *model );
			}
		}
	}
	else
	{
		ImGui::Text( "No results." );
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
	switch( m_ModelsDisplayMode )
	{
		default:
		case List:
			for( int i = 0; i < dir->ModelNames.size(); ++i )
			{
				DrawModelButton( dir->Models[i], dir->ModelNames[i], ImVec2() );
			}
			break;
		case Grid:
			const float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
			const int NumColumns = (int) (ImGui::GetWindowContentRegionWidth() / ButtonSize);
			ImGui::Columns( NumColumns, nullptr, false );
			for( int i = 0; i < dir->ModelNames.size(); ++i )
			{
				DrawModelButton( dir->Models[i], dir->ModelNames[i], ImVec2( m_SpawnmenuButtonSize, m_SpawnmenuButtonSize ) );
				ImGui::NextColumn();
			}
			ImGui::Columns( 1 );
			break;
	}
}

void IcepickMenu::DrawModelButton( std::string & modelName, std::string & displayName, ImVec2 size )
{
	if( ImGui::Button( displayName.c_str() ) )
	{
		DoSpawnModel( modelName );
	}
	if( ImGui::BeginPopupContextItem( modelName.c_str() ) )
	{
		if( ImGui::BeginMenu( "Add to Spawnlist" ) )
		{
			for( Spawnlist & spawnlist : m_ModelsList->Spawnlists )
			{
				if( spawnlist.CanWriteTo && ImGui::MenuItem( spawnlist.Name.c_str() ) )
				{
					m_ModelsList->AppendToSpawnlist( spawnlist, modelName );
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
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

void IcepickMenu::DrawPage( int idx )
{
    const float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
    const int NumColumns = (int)(ImGui::GetWindowContentRegionWidth() / ButtonSize);

	if( SpawnmenuPage * currentPage = &(m_Pages[idx]) )
	{
		for( EntityCategory & entCategory : currentPage->Categories )
		{
			ImGui::SetNextTreeNodeOpen( true, ImGuiCond_FirstUseEver );
			if( ImGui::CollapsingHeader( entCategory.Title.c_str() ) )
			{
				ImGui::Columns( NumColumns, nullptr, false );
				for( SpawnEntity & ent : entCategory.Ents )
				{
					if( ImGui::Button( ent.FriendlyName.c_str(), ImVec2( m_SpawnmenuButtonSize, m_SpawnmenuButtonSize ) ) )
					{
						std::string ExecuteString = entCategory.CallbackName + "( \"" + ent.EntityId + "\" )";
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
					ImGui::NextColumn();
				}
				ImGui::Columns( 1 );
			}
		}
	}
}

void IcepickMenu::DrawSaveAs()
{
	if( !m_SaveAsOpen )
	{
		return;
	}

	ImGui::SetNextWindowSize( ImVec2( 400, 480 ), ImGuiCond_FirstUseEver );
	ImGui::Begin( "Save As", nullptr, ImGuiWindowFlags_NoCollapse );
	{
		ImGui::Text( "Create new save" );
		if( m_ShowInvalidSave )
		{
			ImGui::TextColored( ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ), "Please enter a valid file name." );
		}
		ImGui::InputText( "File name", m_SaveInput, IM_ARRAYSIZE( m_SaveInput ) );

		ImGui::Separator();
		ImGui::Text( "or overwrite an existing save" );

		ImGui::BeginGroup();
		{
			ImGui::BeginChild( "OverwritePane", ImVec2( 0, ImGui::GetWindowHeight() - 140 ) );
			for( std::string & fileName : m_CachedSaveFileNames )
			{
				if( ImGui::Button( fileName.c_str(), ImVec2( ImGui::GetWindowContentRegionWidth(), 0 ) ) )
				{
					strcpy_s( m_SaveInput, fileName.c_str() );
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndGroup();

		if( ImGui::Button( "Save", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.5f - 5, 0 ) ) )
		{
			if( IsSaveInputNameValid() )
			{
				std::string saveFileName = m_SaveInput;
				SDK().GetSQManager().ExecuteServerCode( ( "Spawnmenu_SaveGameToFile(\"" + saveFileName + "\");" ).c_str() );
				SDK().GetSQManager().ExecuteClientCode( ( "Spawnmenu_OnSavedGameToFile(\"" + saveFileName + "\");" ).c_str() );
				m_SaveAsOpen = false;
				m_ShowInvalidSave = false;
			}
			else
			{
				m_ShowInvalidSave = true;
			}
		}
		ImGui::SameLine();
		if( ImGui::Button( "Cancel", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.5f - 5, 0 ) ) )
		{
			m_SaveAsOpen = false;
			m_ShowInvalidSave = false;
		}

	}
	ImGui::End();
}

bool IcepickMenu::IsSaveInputNameValid()
{
	std::vector<char> invalidCharacters = { '/', '\\', '<', '>', '?', ':', '*', '"', '|' };
	std::string input = m_SaveInput;

	for( char invalidCharacter : invalidCharacters )
	{
		if( input.find( invalidCharacter ) != std::string::npos )
		{
			m_logger->warn( "Save name '{}' is invalid, found disallowed character {}", input, invalidCharacter );
			return false;
		}
	}
	return true;
}

void IcepickMenu::RefreshSaveFilesList()
{
	m_CachedSaveFileNames.clear();

	for( auto & dirIter : fs::recursive_directory_iterator( SDK().GetFSManager().GetSavesPath() ) )
	{
		if( dirIter.status().type() != fs::file_type::directory )
		{
			fs::path path = dirIter.path();
			std::string fileString = path.filename().string();
			m_CachedSaveFileNames.push_back( fileString );
		}
	}
}

void IcepickMenu::DrawCreateSpawnlist()
{
	if( !m_CreateSpawnlistOpen )
	{
		return;
	}

	ImGui::SetNextWindowSize( ImVec2( 400, 120 ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowPosCenter();
	ImGui::Begin( "Create Spawnlist", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );
	{
		ImGui::Text( "Create new spawnlist" );
		ImGui::InputText( "File name", m_SpawnlistInput, IM_ARRAYSIZE( m_SpawnlistInput ) );
		ImGui::Separator();

		if( ImGui::Button( "Create", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.5f - 5, 0 ) ) )
		{
			std::string fileName = std::string(m_SpawnlistInput) + ".txt";
			std::string filePath = ( SDK().GetFSManager().GetSpawnlistsPath() / fileName ).string();
			m_logger->info( "create spawnlist at {}", filePath );

			std::ofstream spawnlistFile;
			spawnlistFile.open( filePath );
			spawnlistFile << "";
			spawnlistFile.close();

			m_ModelsList->LoadSpawnlists();

			m_CreateSpawnlistOpen = false;
		}
		ImGui::SameLine();
		if( ImGui::Button( "Cancel", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.5f - 5, 0 ) ) )
		{
			m_CreateSpawnlistOpen = false;
		}
	}
	ImGui::End();
}

void IcepickMenu::DrawCallback()
{
    if (!m_IcepickMenuOpen)
    {
        return;
    }

	float Padding = 10.0f;
	ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x - Padding * 2.0f, ImGui::GetIO().DisplaySize.y - Padding * 2.0f ) );
	ImGui::SetNextWindowPos( ImVec2( Padding, Padding ) );
    ImGui::Begin( "Icepick", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus );
    {
        if (ImGui::BeginMenuBar())
        {
			if( ImGui::MenuItem( "Props" ) )
			{
				m_DisplayingPage = 0;
			}
			for( int i = 0; i < m_Pages.size(); ++i )
			{
				if( ImGui::MenuItem( m_Pages[i].FriendlyName.c_str() ) )
				{
					m_DisplayingPage = i + 1;
				}
			}

			if( ImGui::BeginMenu( "Spawnlists" ) )
			{
				if( ImGui::BeginMenu( "Spawnlist Style" ) )
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
				ImGui::Separator();

				for( Spawnlist & spawnlist : m_ModelsList->Spawnlists )
				{
					if( ImGui::MenuItem( spawnlist.Name.c_str() ) )
					{
						m_ModelsList->ChangeSpawnlist( spawnlist.Name );
					}
				}

				ImGui::Separator();
				if( ImGui::MenuItem( "Create New Spawnlist" ) )
				{
					m_CreateSpawnlistOpen = true;
				}
				ImGui::EndMenu();
			}

            if (ImGui::BeginMenu("Options"))
            {
				if( ImGui::MenuItem( "Edit Mode", nullptr, m_EditModeEnabled ) )
				{
					SDK().GetSQManager().ExecuteClientCode( "Spawnmenu_ToggleEditMode();" );
				}
				if( ImGui::MenuItem( "Invincibility", nullptr, m_IsInvincibilityEnabled ) )
				{
					m_IsInvincibilityEnabled = !m_IsInvincibilityEnabled;
					SDK().GetSQManager().ExecuteServerCode( m_IsInvincibilityEnabled ? "Spawnmenu_ChangePlayerInvincibility( true );" : "Spawnmenu_ChangePlayerInvincibility( false );" );
				}
				if( ImGui::MenuItem( "Save Game" ) )
				{
					RefreshSaveFilesList();
					m_SaveAsOpen = true;
				}
				if( ImGui::MenuItem( "Checkpoint" ) )
				{
					SDK().GetSQManager().ExecuteServerCode( "Spawnmenu_SaveCheckpoint();" );
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
			if( m_DisplayingPage == 0 )
			{
				DrawPropsGui();
			}
			else
			{
				DrawPage( m_DisplayingPage - 1 );
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

	DrawSaveAs();
	DrawCreateSpawnlist();

	// HACK: calling OpenPopup from a squirrel function crashes the game, so we'll fake it using a flag
	if( m_ShowLoadingModal )
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x - Padding * 2.0f, ImGui::GetIO().DisplaySize.y - Padding * 2.0f ) );
		ImGui::SetNextWindowPos( ImVec2( Padding, Padding ) );
		ImGui::Begin( "Loading", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse );
		ImGui::End();

		ImGui::SetNextWindowPos( ImVec2( io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f ), ImGuiCond_Appearing, ImVec2( 0.5f, 0.5f ) );
		ImGui::Begin( "Loading save...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse );
		{
			ImGui::Text( "Please wait while the save file is loaded..." );
		}
		ImGui::End();
	}
	
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

SQInteger IcepickMenu::IsMenuShowing( HSQUIRRELVM v )
{
	sq_pushinteger.CallClient( v, (int) m_IcepickMenuOpen );
	return 1;
}

void IcepickMenu::ToggleToolgunCommand( const CCommand& args )
{
	m_EditModeEnabled = !m_EditModeEnabled;
}

void IcepickMenu::DummyConCommand( const CCommand& args )
{
	// Dummy function, do nothing
}

SQInteger IcepickMenu::ClearTools( HSQUIRRELVM v )
{
	m_Tools.clear();
	return 0;
}
