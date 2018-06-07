#include "stdafx.h"

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

UIManager::UIManager(ConCommandManager& conCommandManager, SquirrelManager& sqManager) :
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

    InitImGui();

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

void UIManager::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // TODO: Make this a bit more reliable
    HWND wnd = FindWindow(NULL, L"Titanfall 2");
    m_logger->info("Game window = {}", (void*)wnd);

    ImGui_ImplDX11_Init(wnd, *m_ppD3D11Device, *m_ppD3D11DeviceContext);
    ImGui::StyleColorsDark();
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
    m_surface->m_vtable->SetCursor(m_surface, dc_arrow);
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
    static float f = 0.0f;
    static int counter = 0;
    ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
    if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
    {
        SDK().GetSQManager().ExecuteServerCode("print(\"called from server\")");
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int UIManager::WindowProcHook(void* game, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg != WM_SETCURSOR && (m_enableCursor || uMsg < WM_KEYFIRST || uMsg > WM_KEYLAST))
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    }
    
    if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
    {
        if (ImGui::GetIO().WantCaptureMouse)
        {
            return 0;
        }
    }

    if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
    {
        if (ImGui::GetIO().WantCaptureKeyboard)
        {
            return 0;
        }
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
    if (!m_enableCursor)
    {
        ISurface_SetCursor(surface, cursor);
    }
    else
    {
        ImGui_UpdateMouseCursor(surface);
    }
}

void UIManager::LockCursorHook(ISurface* surface)
{
    if (!m_enableCursor)
    {
        ISurface_LockCursor(surface);
    }
}

HRESULT UIManager::PresentHook(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)
{
    ImGui_ImplDX11_NewFrame();
    DrawGUI();

    ImVec4 clearColor = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return IDXGISwapChain_Present(SwapChain, SyncInterval, Flags);
}
