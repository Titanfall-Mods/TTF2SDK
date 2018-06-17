#pragma once

enum CursorCode
{
    dc_user,
    dc_none,
    dc_arrow,
    dc_ibeam,
    dc_hourglass,
    dc_waitarrow,
    dc_crosshair,
    dc_up,
    dc_sizenwse,
    dc_sizenesw,
    dc_sizewe,
    dc_sizens,
    dc_sizeall,
    dc_no,
    dc_hand,
    dc_blank,
    dc_last,
};

enum SpawnlistTab
{
	Props,
	Entities,
	Weapons
};

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
	ToolOption( const char * inId, const char * inName, ToolOptionType inType )
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
	Tool( const char * inId, const char * inName, const char * inDesc )
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
	SpawnEntity( char * inName, char * inId )
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
	EntityCategory( char * inTitle, SpawnlistTab inTab, char * inSpawnCode )
	{
		Title = inTitle;
		Tab = inTab;
		SpawnCode = inSpawnCode;
	}

	std::string Title;
	SpawnlistTab Tab;
	std::string SpawnCode;
	std::vector<SpawnEntity> Ents;
	ExecutionContext Context = CONTEXT_SERVER;
};

class UIManager
{
public:
    UIManager(ConCommandManager& conCommandManager, SquirrelManager& sqManager);
    ~UIManager();

    void InitImGui();

    void ShowCursorCommand(const CCommand& args);
	void RegisterToolCommand( const CCommand& args );
	void RegisterToolOption( const CCommand& args );
	void SetOptionDefaultValue( const CCommand& args );
	void SetOptionMinMax( const CCommand& args );

	Tool * GetToolFromId( const char * toolId );
	ToolOption * GetOptionFromId( Tool * tool, const char * optionId );

    SQInteger SQShowCursor(HSQUIRRELVM v);
    SQInteger SQHideCursor(HSQUIRRELVM v);

    void DrawGUI();
    void AddDrawCallback(const std::string& name, const std::function<void()>& func);
    void RemoveDrawCallback(const std::string& name);

    bool IsACursorVisible();

	void DrawPropsGui();
	void DrawModelsDirectory( struct ModelsDirectory * dir );
	void DrawDirectoryModels( struct ModelsDirectory * dir );
	void DrawToolsGui( float ToolsPanelWidth );
	void DrawOptionsGui();
	void DrawCategoryTab( SpawnlistTab displayTab );

	void DoSpawnModel( std::string & model );

    int WindowProcHook(void* game, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void SetCursorHook(ISurface* surface, unsigned int cursor);
    void LockCursorHook(ISurface* surface);
    HRESULT STDMETHODCALLTYPE PresentHook(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);

private:
    std::shared_ptr<spdlog::logger> m_logger;

    ID3D11Device** m_ppD3D11Device = nullptr;
    ID3D11DeviceContext** m_ppD3D11DeviceContext = nullptr;
    IDXGISwapChain** m_ppSwapChain = nullptr;
    ID3D11RenderTargetView* m_guiRenderTargetView = nullptr;

    std::map<std::string, std::function<void()>> m_drawCallbacks;
    SourceInterface<ISurface> m_surface;

    std::atomic_bool m_enableCursor;
    std::atomic_bool m_engineCursorSet;

	std::vector<Tool> m_Tools;
	Tool * m_ViewingTool = nullptr;

	std::vector<EntityCategory> m_EntCategories;
	SpawnlistTab m_DisplayingTab = SpawnlistTab::Props;
	bool m_IcepickMenuOpen = false;

	int m_SpawnmenuButtonSizes[7] = { 0, 32, 48, 64, 96, 128, 256 };
	int m_SpawnmenuButtonSize = 96;

	class ModelsList * m_ModelsList = nullptr;
	struct ModelsDirectory * m_ViewingDirectory = nullptr;
	SpawnlistDisplayMode m_SpawnlistDisplayMode = SpawnlistDisplayMode::Tree;
	ModelsDisplayMode m_ModelsDisplayMode = ModelsDisplayMode::List;

};
