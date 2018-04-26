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

//#undef __cplusplus
#define D3D11_NO_HELPERS
#define CINTERFACE
#include <d3d11.h>
#undef CINTERFACE
//#define __cplusplus

#include <shared_mutex>
#include <unordered_set>
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
#include "Squirrel.h"
#include "IFileSystem.h"
#include "IEngineClient.h"
#include "IEngineServer.h"
#include "VTableHooking.h"
#include "TTF2SDK.h"
