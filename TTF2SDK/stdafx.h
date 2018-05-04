#pragma once

#include "targetver.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <TlHelp32.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#define D3D11_NO_HELPERS
#define CINTERFACE
#include <d3d11.h>
#undef CINTERFACE

#include <d3dcompiler.h>

// TODO: clean these up
#include <algorithm>
#include <shared_mutex>
#include <unordered_set>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <string>
#include <fstream>
#include <regex>
#include <mutex>
#include <MinHook/MinHook.h>
#include <sstream>
#include <DbgHelp.h>
#include "Console.h"
#include "ModuleScan.h"
#include "SigScanning.h"
#include "SourceInterface.h"
#include "Util.h"
#include "IFileSystem.h"
#include "IVEngineServer.h"
#include "IVEngineClient.h"
#include "VTableHooking.h"
#include "ConCommandManager.h"
#include "SquirrelManager.h"
#include "FileSystemManager.h"
#include "TTF2SDK.h"
