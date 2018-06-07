#pragma once

struct DelayedFunc
{
    int FramesTilRun;
    std::function<void()> Func;

    DelayedFunc(int frames, std::function<void()> func)
    {
        FramesTilRun = frames;
        Func = func;
    }
};

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
    std::list<DelayedFunc> m_delayedFuncs; // TODO: Probably not the right data structure

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

    void AddDelayedFunc(std::function<void()> func, int frames);

    void RunFrameHook(double absTime, float frameTime);

    void compileShaders();
};

TTF2SDK& SDK();
bool SetupSDK(const SDKSettings& settings);
void FreeSDK();
