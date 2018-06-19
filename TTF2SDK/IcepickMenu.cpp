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

	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "ClearTools", "", "Help text", WRAPPED_MEMBER( ClearTools ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "RegisterTool", "string id, string friendlyName, string tooltip", "Help text", WRAPPED_MEMBER( RegisterTool ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "AddDividerOption", "string toolId", "Help text", WRAPPED_MEMBER( AddToolOption_Divider ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "AddTextOption", "string toolId, string text", "Help text", WRAPPED_MEMBER( AddToolOption_Text ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "AddButtonOption", "string toolId, string id, string buttonText", "Help text", WRAPPED_MEMBER( AddToolOption_Button ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "AddSliderOption", "string toolId, string id, string name, float default, float min, float max", "Help text", WRAPPED_MEMBER( AddToolOption_Slider ) );
	sqManager.AddFuncRegistration( CONTEXT_SERVER, "void", "AddIntSliderOption", "string toolId, string id, string name, int default, int min, int max", "Help text", WRAPPED_MEMBER( AddToolOption_IntSlider ) );

	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "ClearSpawnmenu", "", "Help text", WRAPPED_MEMBER( ClearSpawnmenu ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "RegisterSpawnmenuPage", "string id, string friendlyName", "Help text", WRAPPED_MEMBER( RegisterSpawnmenuPage ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "RegisterPageCategory", "string pageId, string id, string friendlyName, string callbackName", "Help text", WRAPPED_MEMBER( RegisterPageCategory ) );
	sqManager.AddFuncRegistration( CONTEXT_CLIENT, "void", "RegisterCategoryItem", "string categoryId, string itemId, string friendlyName", "Help text", WRAPPED_MEMBER( RegisterCategoryItem ) );

    // Add models
    m_ModelsList = new ModelsList();
    m_ViewingDirectory = &m_ModelsList->BaseDir;

    uiManager.AddDrawCallback("IcepickMenu", std::bind(&IcepickMenu::DrawCallback, this));
}

IcepickMenu::~IcepickMenu()
{
    SDK().GetUIManager().RemoveDrawCallback("IcepickMenu");
}

SQInteger IcepickMenu::RegisterTool( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallClient( v, 1 );
	const SQChar * name = sq_getstring.CallClient( v, 2 );
	const SQChar * tooltip = sq_getstring.CallClient( v, 3 );

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
	const SQChar * toolId = sq_getstring.CallClient( v, 1 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		tool->Options.push_back( ToolOption( toolId, "", ToolOptionType::Divider ) );
	}
	return 0;
}

SQInteger IcepickMenu::AddToolOption_Text( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallClient( v, 1 );
	const SQChar * text = sq_getstring.CallClient( v, 2 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		tool->Options.push_back( ToolOption( text, text, ToolOptionType::Text ) );
	}
	return 0;
}

SQInteger IcepickMenu::AddToolOption_Button( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallClient( v, 1 );
	const SQChar * optionId = sq_getstring.CallClient( v, 2 );
	const SQChar * text = sq_getstring.CallClient( v, 3 );

	if( Tool * tool = GetToolFromId( toolId ) )
	{
		tool->Options.push_back( ToolOption( optionId, text, ToolOptionType::Button ) );
	}
	return 0;
}

SQInteger IcepickMenu::AddToolOption_Slider( HSQUIRRELVM v )
{
	const SQChar * toolId = sq_getstring.CallClient( v, 1 );
	const SQChar * optionId = sq_getstring.CallClient( v, 2 );
	const SQChar * name = sq_getstring.CallClient( v, 3 );
	float defaultValue = sq_getfloat.CallClient( v, 4 );
	float min = sq_getfloat.CallClient( v, 5 );
	float max = sq_getfloat.CallClient( v, 6 );

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
	const SQChar * toolId = sq_getstring.CallClient( v, 1 );
	const SQChar * optionId = sq_getstring.CallClient( v, 2 );
	const SQChar * name = sq_getstring.CallClient( v, 3 );
	int defaultValue = sq_getinteger.CallClient( v, 4 );
	int min = sq_getinteger.CallClient( v, 5 );
	int max = sq_getinteger.CallClient( v, 6 );

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
		spdlog::get( "logger" )->error( "Could not register category! No page found with id {}", pageId );
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
		spdlog::get( "logger" )->error( "Could not register item! No category found with id {}", categoryId );
	}

	return 0;
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

void IcepickMenu::DrawPage( int idx )
{
    const float ButtonSize = m_SpawnmenuButtonSize > 0 ? m_SpawnmenuButtonSize : 200.0f;
    const int NumColumns = (int)(ImGui::GetWindowContentRegionWidth() / ButtonSize);

	if( SpawnmenuPage * currentPage = &(m_Pages[idx]) )
	{
		for( EntityCategory & entCategory : currentPage->Categories )
		{
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

void IcepickMenu::DrawCallback()
{
    if (!m_IcepickMenuOpen)
    {
        return;
    }

	float Padding = 10.0f;
	ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x - Padding * 2.0f, ImGui::GetIO().DisplaySize.y - Padding * 2.0f ) );
	ImGui::SetNextWindowPos( ImVec2( Padding, Padding ) );
    ImGui::Begin( "Icepick", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse );
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

SQInteger IcepickMenu::ClearTools( HSQUIRRELVM v )
{
	m_Tools.clear();
	return 0;
}
