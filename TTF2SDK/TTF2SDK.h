#pragma once

class TTF2SDK
{
private:
    std::shared_ptr<spdlog::logger> m_logger;

    std::unique_ptr<FileSystemManager> m_fsManager;
    std::unique_ptr<ConCommandManager> m_conCommandManager;
    std::unique_ptr<SquirrelManager> m_sqManager;
    std::unique_ptr<PakManager> m_pakManager;
    std::unique_ptr<ModManager> m_modManager;
    std::unique_ptr<UIManager> m_uiManager;

    std::list<std::unique_ptr<IFrameTask>> m_frameTasks;

    SourceInterface<IVEngineServer> m_engineServer;
    SourceInterface<IVEngineClient> m_engineClient;

public:
    TTF2SDK(const SDKSettings& settings);
    ~TTF2SDK();

    FileSystemManager& GetFSManager();
    ConCommandManager& GetConCommandManager();
    SquirrelManager& GetSQManager();
    PakManager& GetPakManager();
    ModManager& GetModManager();
    UIManager& GetUIManager();

    SourceInterface<IVEngineServer>& GetEngineServer();
    SourceInterface<IVEngineClient>& GetEngineClient();

    void RunFrameHook(double absTime, float frameTime);

    void AddFrameTask(std::unique_ptr<IFrameTask> task);

    void compileShaders();
};

TTF2SDK& SDK();
bool SetupSDK(const SDKSettings& settings);
void FreeSDK();
