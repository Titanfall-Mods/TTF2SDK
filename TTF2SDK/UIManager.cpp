#include "stdafx.h"
#include <regex>
#include <algorithm>
#include <string>

UIManager& UIMan()
{
    return SDK().GetUIManager();
}

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&UIManager::##name), &UIManager::##name, decltype(&UIMan), &UIMan>::Call

SigScanFunc<void> d3d11DeviceFinder("materialsystem_dx11.dll", "\x48\x83\xEC\x00\x33\xC0\x89\x54\x24\x00\x4C\x8B\xC9\x48\x8B\x0D\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00", "xxx?xxxxx?xxxxxx????xxx?????");
SigScanFunc<void> d3d11ContextFinder("materialsystem_dx11.dll", "\x40\x53\x48\x83\xEC\x00\x48\x8B\x0D\x00\x00\x00\x00\x48\x8B\x01\xFF\x90\x00\x00\x00\x00\xE8\x00\x00\x00\x00", "xxxxx?xxx????xxxxx????x????");
SigScanFunc<void> d3d11SwapChainFinder("materialsystem_dx11.dll", "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x83\x3D\x00\x00\x00\x00\x00\x8B\xDA", "xxxx?xxxx?xxx?????xx");

HookedVTableFunc<decltype(&IDXGISwapChainVtbl::Present), &IDXGISwapChainVtbl::Present> IDXGISwapChain_Present;

HookedFunc<int, void*, HWND, UINT, WPARAM, LPARAM> GameWindowProc("inputsystem.dll", "\x48\x89\x54\x24\x00\x55\x56\x41\x55", "xxxx?xxxx");

HookedVTableFunc<decltype(&ISurface::VTable::LockCursor), &ISurface::VTable::LockCursor> ISurface_LockCursor;
HookedVTableFunc<decltype(&ISurface::VTable::SetCursor), &ISurface::VTable::SetCursor> ISurface_SetCursor;

UIManager::UIManager(ConCommandManager& conCommandManager, SquirrelManager& sqManager, FileSystemManager& fsManager) :
    m_surface("vguimatsurface.dll", "VGUI_Surface031")
{
    m_logger = spdlog::get("logger");

    // Get pointer to d3d device
    char* funcBase = (char*)d3d11DeviceFinder.GetFuncPtr();
    int offset = *(int*)(funcBase + 16);
    m_ppD3D11Device = (ID3D11Device**)(funcBase + 20 + offset);

    // Get pointer to d3d context
    funcBase = (char*)d3d11ContextFinder.GetFuncPtr();
    offset = *(int*)(funcBase + 9);
    m_ppD3D11DeviceContext = (ID3D11DeviceContext**)(funcBase + 13 + offset);

    // Get the swap chain
    funcBase = (char*)d3d11SwapChainFinder.GetFuncPtr();
    offset = *(int*)(funcBase + 13);
    m_ppSwapChain = (IDXGISwapChain**)(funcBase + 18 + offset);

    SPDLOG_DEBUG(m_logger, "m_ppD3D11Device = {}", (void*)m_ppD3D11Device);
    SPDLOG_DEBUG(m_logger, "pD3D11Device = {}", (void*)*m_ppD3D11Device);

    SPDLOG_DEBUG(m_logger, "m_ppD3D11DeviceContext = {}", (void*)m_ppD3D11DeviceContext);
    SPDLOG_DEBUG(m_logger, "pD3D11DeviceContext = {}", (void*)*m_ppD3D11DeviceContext);

    SPDLOG_DEBUG(m_logger, "m_ppSwapChain = {}", (void*)m_ppSwapChain);
    SPDLOG_DEBUG(m_logger, "pSwapChain = {}", (void*)*m_ppSwapChain);

    InitImGui(fsManager.GetModsPath());

    IDXGISwapChain_Present.Hook((*m_ppSwapChain)->lpVtbl, WRAPPED_MEMBER(PresentHook));

    GameWindowProc.Hook(WRAPPED_MEMBER(WindowProcHook));
    ISurface_SetCursor.Hook(m_surface->m_vtable, WRAPPED_MEMBER(SetCursorHook));
    ISurface_LockCursor.Hook(m_surface->m_vtable, WRAPPED_MEMBER(LockCursorHook));

    conCommandManager.RegisterCommand("show_cursor", WRAPPED_MEMBER(ShowCursorCommand), "Set visibility of cursor", 0);

    sqManager.AddFuncRegistration(CONTEXT_CLIENT, "ShowCursor", WRAPPED_MEMBER(SQShowCursor));
    sqManager.AddFuncRegistration(CONTEXT_CLIENT, "HideCursor", WRAPPED_MEMBER(SQHideCursor));
}

UIManager::~UIManager()
{
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
}

void UIManager::InitImGui(const fs::path& modsPath)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // TODO: Make this a bit more reliable
    HWND wnd = FindWindow(NULL, L"Titanfall 2");
    m_logger->info("Game window = {}", (void*)wnd);

    ImGui_ImplDX11_Init(wnd, *m_ppD3D11Device, *m_ppD3D11DeviceContext);
    ImGui::StyleColorsDark();

    // Check if the font file exists in the icepick mod
    fs::path fontPath = modsPath / "Icepick.Framework/fonts/NotoSans-Medium.ttf";
    if (fs::exists(fontPath))
    {
        ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 16.0f);
    }
    
    ImGui_ImplDX11_CreateDeviceObjects();
}

void UIManager::ShowCursorCommand(const CCommand& args)
{
    if (strcmp(args[1], "1") == 0)
    {
        SQShowCursor(0); // TODO: Fix this
    }
    else if (strcmp(args[1], "0") == 0)
    {
        SQHideCursor(0); // TODO: Fix this
    }
    else
    {
        m_logger->error("Invalid argument to show_cursor, must be 1 or 0");
    }
}

SQInteger UIManager::SQShowCursor(HSQUIRRELVM v)
{
    m_enableCursor = true;
    m_logger->info("Showing cursor");
    m_surface->m_vtable->UnlockCursor(m_surface);
    ISurface_SetCursor(m_surface, dc_arrow);
    return 0;
}

SQInteger UIManager::SQHideCursor(HSQUIRRELVM v)
{
    m_enableCursor = false;
    m_logger->info("Hiding cursor");
    return 0;
}

void UIManager::DrawGUI()
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    if (corner != -1)
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
    if (ImGui::Begin("DebugOverlay", nullptr, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
        ImGui::Text("m_enableCursor: %d", (int)m_enableCursor);
        ImGui::Text("m_engineCursorSet: %d", (int)m_engineCursorSet);
        ImGui::Text("IsCursorVisible: %d", m_surface->m_vtable->IsCursorVisible(m_surface));
        ImGui::Text("WantCaptureMouse: %d", ImGui::GetIO().WantCaptureMouse);
        ImGui::Text("WantCaptureKeyboard: %d", ImGui::GetIO().WantCaptureKeyboard);
        ImGui::End();
    }
}

void UIManager::AddDrawCallback(const std::string& name, const std::function<void()>& func)
{
    m_drawCallbacks[name] = func;
}

void UIManager::RemoveDrawCallback(const std::string& name)
{
    m_drawCallbacks.erase(name);
}

bool IsKeyMsg(UINT uMsg)
{
    return uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST;
}

bool IsMouseMsg(UINT uMsg)
{
    return uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST;
}

bool UIManager::IsACursorVisible()
{
    return m_enableCursor || m_engineCursorSet;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int UIManager::WindowProcHook(void* game, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Don't pass to imgui if there's no cursor visible
    if (uMsg == WM_SETCURSOR || !IsACursorVisible())
    {
        return GameWindowProc(game, hWnd, uMsg, wParam, lParam);
    }

    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    
    // Do not pass to game if we're forcing the cursor
    bool forcedCursor = m_enableCursor && !m_engineCursorSet;

    // Only block from game if imgui capturing
    if (IsMouseMsg(uMsg) && (forcedCursor || ImGui::GetIO().WantCaptureMouse))
    {
        return 0;
    }
    
    if (IsKeyMsg(uMsg) && ImGui::GetIO().WantCaptureKeyboard)
    {
        return 0;
    }

    return GameWindowProc(game, hWnd, uMsg, wParam, lParam);
}

static bool ImGui_UpdateMouseCursor(ISurface* surface)
{
    ImGuiIO& io = ImGui::GetIO();
    //if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
    //    return false;

    ImGuiMouseCursor imgui_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        ISurface_SetCursor(surface, dc_none);
    }
    else
    {
        // Hardware cursor type
        unsigned int cursor = dc_arrow;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        cursor = dc_arrow; break;
        case ImGuiMouseCursor_TextInput:    cursor = dc_ibeam; break;
        case ImGuiMouseCursor_ResizeAll:    cursor = dc_sizeall; break;
        case ImGuiMouseCursor_ResizeEW:     cursor = dc_sizewe; break;
        case ImGuiMouseCursor_ResizeNS:     cursor = dc_sizens; break;
        case ImGuiMouseCursor_ResizeNESW:   cursor = dc_sizenesw; break;
        case ImGuiMouseCursor_ResizeNWSE:   cursor = dc_sizenwse; break;
        }
        ISurface_SetCursor(surface, cursor);
    }
    return true;
}

void UIManager::SetCursorHook(ISurface* surface, unsigned int cursor)
{
    m_engineCursorSet = (cursor != dc_user && cursor != dc_none && cursor != dc_blank);

    // If no cursors, let the engine deal with it
    if (!IsACursorVisible())
    {
        return ISurface_SetCursor(surface, cursor);
    }

    // If there's a cursor, and ImGui is capturing, let that handle it
    if (ImGui::GetIO().WantCaptureMouse)
    {
        ImGui_UpdateMouseCursor(surface);
        return;
    }

    // If we're forcing the cursor and the engine isn't, use ours
    if (m_enableCursor && !m_engineCursorSet)
    {
        ImGui_UpdateMouseCursor(surface);
        return;
    }
    
    // Otherwise let the game handle it
    ISurface_SetCursor(surface, cursor);
}

void UIManager::LockCursorHook(ISurface* surface)
{
    // Only allow the cursor to be locked if we're not forcing it
    if (!m_enableCursor)
    {
        return ISurface_LockCursor(surface);
    }
}

HRESULT UIManager::PresentHook(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)
{
    ImGui_ImplDX11_NewFrame();
    //DrawGUI();
    for (const auto& entry : m_drawCallbacks)
    {
        entry.second();
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return IDXGISwapChain_Present(SwapChain, SyncInterval, Flags);
}
