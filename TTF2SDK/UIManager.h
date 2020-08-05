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

class UIManager
{
public:
    UIManager(ConCommandManager& conCommandManager, SquirrelManager& sqManager, FileSystemManager& fsManager, ID3D11Device** ppD3DDevice);
    ~UIManager();

    void InitImGui(const fs::path& modsPath, ID3D11Device** ppD3DDevice);
    void ShowCursorCommand(const CCommand& args);
	
    SQInteger SQShowCursor(HSQUIRRELVM v);
    SQInteger SQHideCursor(HSQUIRRELVM v);

    void DrawGUI();
    void AddDrawCallback(const std::string& name, const std::function<void()>& func);
    void RemoveDrawCallback(const std::string& name);

    bool IsACursorVisible();

    void UpdateImGuiKeyStates();

    int WindowProcHook(void* game, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void SetCursorHook(ISurface* surface, unsigned int cursor);
    void LockCursorHook(ISurface* surface);
    HRESULT STDMETHODCALLTYPE PresentHook(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);

private:
    std::shared_ptr<spdlog::logger> m_logger;

    ID3D11DeviceContext** m_ppD3D11DeviceContext = nullptr;
    IDXGISwapChain** m_ppSwapChain = nullptr;
    ID3D11RenderTargetView* m_guiRenderTargetView = nullptr;

    std::map<std::string, std::function<void()>> m_drawCallbacks;
    SourceInterface<ISurface> m_surface;

    std::atomic_bool m_enableCursor;
    std::atomic_bool m_engineCursorSet;
};
