#include "stdafx.h"
#include "SoundManager.h"
#include <iostream>
#include <windows.h>
#include <mmsystem.h>

std::wstring s2ws(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

SoundManager& SoundMan()
{
	return SDK().GetSoundManager();
}
#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&SoundManager::##name), &SoundManager::##name, decltype(&SoundMan), &SoundMan>::Call
SQInteger SoundManager::playsound(HSQUIRRELVM v)
{
	fs::path base = SDK().GetFSManager().GetBasePath();
	const SQChar * filename = sq_getstring.CallServer(v, 1);
	std::string fullfile = base.u8string() + "Sounds\\" + filename;
	std::wstring wfullfile = s2ws(fullfile);
	spdlog::get("logger")->info(fullfile);
	PlaySound((wfullfile.c_str()), NULL, SND_FILENAME | SND_ASYNC);
	return 0;
}

SoundManager::SoundManager(TTF2SDK& sdk, SquirrelManager& squirrel, FileSystemManager& fs)
{
	spdlog::get("logger")->info("SoundPlayer is being loaded");
	squirrel.AddFuncRegistration(CONTEXT_CLIENT, "void", "Playsound", "string filename", "", WRAPPED_MEMBER(playsound));
}

