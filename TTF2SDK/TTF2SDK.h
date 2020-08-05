#pragma once

// TODO: Move into separate header
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

// TODO: Move into separate header
class DelayedFuncTask : public IFrameTask
{
public:
    virtual ~DelayedFuncTask() {}
    virtual void RunFrame()
    {
        for (auto& delay : m_delayedFuncs)
        {
            delay.FramesTilRun = std::max(delay.FramesTilRun - 1, 0);
            if (delay.FramesTilRun == 0)
            {
                delay.Func();
            }
        }

        auto newEnd = std::remove_if(m_delayedFuncs.begin(), m_delayedFuncs.end(), [](const DelayedFunc& delay)
        {
            return delay.FramesTilRun == 0;
        });
        m_delayedFuncs.erase(newEnd, m_delayedFuncs.end());
    }

    virtual bool IsFinished()
    {
        return false;
    }

    void AddFunc(std::function<void()> func, int frames)
    {
        std::lock_guard<std::mutex> l(m_lock);
        m_delayedFuncs.emplace_back(frames, func);
    }

private:
    std::mutex m_lock;
    std::list<DelayedFunc> m_delayedFuncs;
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
    std::unique_ptr<SourceConsole> m_sourceConsole;
	std::unique_ptr<SoundManager> m_soundmanager;

    std::unique_ptr<IcepickMenu> m_icepickMenu;

    std::list<std::shared_ptr<IFrameTask>> m_frameTasks;
    std::shared_ptr<DelayedFuncTask> m_delayedFuncTask;

    SourceInterface<IVEngineServer> m_engineServer;
    SourceInterface<IVEngineClient> m_engineClient;
    SourceInterface<IInputSystem> m_inputSystem;

    ID3D11Device** m_ppD3D11Device;

public:
    TTF2SDK(const SDKSettings& settings);
    ~TTF2SDK();

    FileSystemManager& GetFSManager();
    ConCommandManager& GetConCommandManager();
    SquirrelManager& GetSQManager();
    PakManager& GetPakManager();
    ModManager& GetModManager();
    UIManager& GetUIManager();
    SourceConsole& GetSourceConsole();
	SoundManager& GetSoundManager();

    ID3D11Device** GetD3D11DevicePtr();

    IcepickMenu& GetIcepickMenu();

    SourceInterface<IVEngineServer>& GetEngineServer();
    SourceInterface<IVEngineClient>& GetEngineClient();
    SourceInterface<IInputSystem>& GetInputSystem();

    void RunFrameHook(double absTime, float frameTime);

    void AddFrameTask(std::shared_ptr<IFrameTask> task);
    void AddDelayedFunc(std::function<void()> func, int frames);

    SQInteger SQGetMouseDeltaX(HSQUIRRELVM v);
    SQInteger SQGetMouseDeltaY(HSQUIRRELVM v);

    void EnableNoclipCommand(const CCommand& args);
    void DisableNoclipCommand(const CCommand& args);
};

TTF2SDK& SDK();
bool SetupSDK(const SDKSettings& settings);
void FreeSDK();
extern std::unique_ptr<Console> g_console;
