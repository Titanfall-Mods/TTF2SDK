#pragma once

class TTF2SDK
{
private:
    std::shared_ptr<spdlog::logger> m_logger;

    std::unique_ptr<FileSystemManager> m_fsManager;
    std::unique_ptr<ConCommandManager> m_conCommandManager;
    std::unique_ptr<SquirrelManager> m_sqManager;
    
    SourceInterface<IVEngineServer> m_engineServer;
    SourceInterface<IVEngineClient> m_engineClient;

public:
    ID3D11Device** m_ppD3D11Device = nullptr;

    TTF2SDK();
    ~TTF2SDK();

    FileSystemManager& GetFSManager();
    SquirrelManager& GetSQManager();
    ConCommandManager& GetConCommandManager();

    SourceInterface<IVEngineServer>& GetEngineServer();
    SourceInterface<IVEngineClient>& GetEngineClient();

    void RunFrameHook(double absTime, float frameTime);

    void compileShaders();
};

TTF2SDK& SDK();
bool SetupSDK();
void FreeSDK();
