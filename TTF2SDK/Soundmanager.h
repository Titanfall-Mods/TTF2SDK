#pragma once
#include "stdafx.h"
class TTF2SDK;

class SoundManager {
public:
	SQInteger playsound(HSQUIRRELVM v);
	SoundManager(TTF2SDK& sdk, SquirrelManager& squirrel, FileSystemManager& fs);
};
