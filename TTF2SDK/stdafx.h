#pragma once

#include "targetver.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef DELETE

#include <TlHelp32.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#define D3D11_NO_HELPERS
#define CINTERFACE
#define INITGUID
#include <d3d11.h>
#include <DXGI.h>
#undef INITGUID
#undef CINTERFACE

#include <d3dcompiler.h>
#include "proto/pak_cache.pb.h"

// imgui
#include <imgui/imgui.h>
#include <imgui/examples/directx11_example/imgui_impl_dx11.h>

// TODO: clean these up
#include "Memory.h"
#include "tier0.h"
#include <algorithm>
#include <functional>
#include <shared_mutex>
#include <unordered_set>
#include <filesystem>
namespace fs = std::experimental::filesystem;
#include <locale>
#include <codecvt>
#include <numeric>
#include <string>
#include <fstream>
#include <regex>
#include <mutex>
#include <MinHook/MinHook.h>
#include <sstream>
#include "SDKSettings.h"
#include "Console.h"
#include "ModuleScan.h"
#include "SigScanning.h"
#include "SourceInterface.h"
#include "Util.h"
#include "IFrameTask.h"
#include "IFileSystem.h"
#include "ISurface.h"
#include "IVEngineServer.h"
#include "IVEngineClient.h"
#include "IVModelInfo.h"
#include "ICvar.h"
#include "IInputSystem.h"
#include "VTableHooking.h"
#include "ConCommandManager.h"
#include "SquirrelManager.h"
#include "FileSystemManager.h"
#include "ModManager.h"
#include "PakManager.h"
#include "Preloader.h"
#include "UIManager.h"
#include "IcepickMenu.h"
#include "CrashReporting.h"
#include "TTF2SDK.h"

