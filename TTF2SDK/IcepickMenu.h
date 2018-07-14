#pragma once

#define SearchMaxChars 255

enum SpawnlistDisplayMode
{
    Tree,
    Browser
};

enum ModelsDisplayMode
{
    List,
    Grid
};

enum ToolOptionType
{
    Divider = -1,
    Button = 0,
    Slider,
    IntSlider,
    Text,
};

struct ToolOption
{
public:
    ToolOption(const char * inId, const char * inName, ToolOptionType inType)
    {
        Id = inId;
        FriendlyName = inName;
        Type = inType;
    }

    std::string Id;
    std::string FriendlyName;
    ToolOptionType Type;

    float FloatValue = 0.0f;
    int IntValue = 0;
    std::string StringValue;

    int Min = 0;
    int Max = 10;
};

struct Tool
{
public:
    Tool(const char * inId, const char * inName, const char * inDesc)
    {
        Id = inId;
        FriendlyName = inName;
        Description = inDesc;
    }

    std::string Id;
    std::string FriendlyName;
    std::string Description;
    std::vector<ToolOption> Options;
};

struct SpawnEntity
{
public:
    SpawnEntity( const char * inName, const char * inId )
    {
        FriendlyName = inName;
        EntityId = inId;
    }

    std::string FriendlyName;
    std::string EntityId;
};

struct EntityCategory
{
public:
	EntityCategory( const char * inId, const char * inTitle, const char * inCallbackName )
	{
		Id = inId;
		Title = inTitle;
		CallbackName = inCallbackName;
	}

	std::string Id;
	std::string Title;
	std::string CallbackName;
	std::vector<SpawnEntity> Ents;
	ExecutionContext Context = CONTEXT_SERVER;
};

struct SpawnmenuPage
{
public:
	SpawnmenuPage( const char * inId, const char * inFriendlyName )
	{
		Id = inId;
		FriendlyName = inFriendlyName;
	}

	std::string Id;
	std::string FriendlyName;
	std::vector<EntityCategory> Categories;
};

class IcepickMenu
{
public:
    IcepickMenu(ConCommandManager& conCommandManager, UIManager& uiManager, SquirrelManager& sqManager);
    ~IcepickMenu();

    void DrawPropsGui();
	void DrawSearchResults();
    void DrawModelsDirectory(struct ModelsDirectory * dir);
    void DrawDirectoryModels(struct ModelsDirectory * dir);
    void DrawToolsGui(float ToolsPanelWidth);
    void DrawOptionsGui();
    void DrawPage( int idx );

    void DrawCallback();

    void DoSpawnModel(std::string & model);

    void ShowMenuCommand(const CCommand& args);
    void HideMenuCommand(const CCommand& args);

	SQInteger ClearTools( HSQUIRRELVM v );
	SQInteger RegisterTool( HSQUIRRELVM v );
	SQInteger AddToolOption_Divider( HSQUIRRELVM v );
	SQInteger AddToolOption_Text( HSQUIRRELVM v );
	SQInteger AddToolOption_Button( HSQUIRRELVM v );
	SQInteger AddToolOption_Slider( HSQUIRRELVM v );
	SQInteger AddToolOption_IntSlider( HSQUIRRELVM v );

	SQInteger ClearSpawnmenu( HSQUIRRELVM v );
	SQInteger RegisterSpawnmenuPage( HSQUIRRELVM v );
	SQInteger RegisterPageCategory( HSQUIRRELVM v );
	SQInteger RegisterCategoryItem( HSQUIRRELVM v );

	SQInteger ClearSaveBuffer( HSQUIRRELVM v );
	SQInteger AddSaveItem( HSQUIRRELVM v );
	SQInteger WriteSaveBufferToFile( HSQUIRRELVM v );

	SQInteger GetSaveFiles( HSQUIRRELVM v );
	SQInteger LoadSaveFileContents( HSQUIRRELVM v );

	SQInteger EnableEditMode( HSQUIRRELVM v );
	SQInteger DisableEditMode( HSQUIRRELVM v );
	SQInteger IsEditModeEnabled( HSQUIRRELVM v );

    Tool * GetToolFromId(const char * toolId);
    ToolOption * GetOptionFromId(Tool * tool, const char * optionId);
	SpawnmenuPage * GetPageFromId( const char * pageId );
	EntityCategory * GetCategoryFromId( const char * categoryId );

protected:

	void ClearSearch();

	int SearchInputLength();

	void UpdateSearchResults();

private:
    std::shared_ptr<spdlog::logger> m_logger;

    std::vector<Tool> m_Tools;
    Tool * m_ViewingTool = nullptr;

	int m_DisplayingPage = 0;
	std::vector<SpawnmenuPage> m_Pages;
    bool m_IcepickMenuOpen = false;
	bool m_EditModeEnabled = true;

    int m_SpawnmenuButtonSizes[7] = { 0, 32, 48, 64, 96, 128, 256 };
    int m_SpawnmenuButtonSize = 96;

	char m_SearchInput[SearchMaxChars] = "";
	int m_CachedSearchInputLength = 0;
	std::vector<std::string *> m_SearchResults;

    class ModelsList * m_ModelsList = nullptr;
    struct ModelsDirectory * m_ViewingDirectory = nullptr;
    SpawnlistDisplayMode m_SpawnlistDisplayMode = SpawnlistDisplayMode::Tree;
    ModelsDisplayMode m_ModelsDisplayMode = ModelsDisplayMode::List;

	std::vector<std::string> m_SaveBuffer;
};
