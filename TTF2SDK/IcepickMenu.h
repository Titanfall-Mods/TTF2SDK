#pragma once

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
    SpawnEntity(char * inName, char * inId)
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
    EntityCategory(char * inTitle, SpawnlistTab inTab, char * inSpawnCode)
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

class IcepickMenu
{
public:
    IcepickMenu(ConCommandManager& conCommandManager, UIManager& uiManager, SquirrelManager& sqManager);
    ~IcepickMenu();

    void DrawPropsGui();
    void DrawModelsDirectory(struct ModelsDirectory * dir);
    void DrawDirectoryModels(struct ModelsDirectory * dir);
    void DrawToolsGui(float ToolsPanelWidth);
    void DrawOptionsGui();
    void DrawCategoryTab(SpawnlistTab displayTab);

    void DrawCallback();

    void DoSpawnModel(std::string & model);

    void ShowMenuCommand(const CCommand& args);
    void HideMenuCommand(const CCommand& args);

    void RegisterToolCommand(const CCommand& args);
    void RegisterToolOption(const CCommand& args);
    void SetOptionDefaultValue(const CCommand& args);
    void SetOptionMinMax(const CCommand& args);

    Tool * GetToolFromId(const char * toolId);
    ToolOption * GetOptionFromId(Tool * tool, const char * optionId);

    SQInteger ExampleClientFunc(HSQUIRRELVM v);

private:
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
