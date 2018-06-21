#pragma once

#include <string>

struct ModelsDirectory
{
	std::string FullPath;
	std::string Path;
	std::map<std::string, ModelsDirectory> SubDirectories;
	std::vector<std::string> Models;
	std::vector<std::string> ModelNames;
};

class ModelsList
{

public:

	static std::vector<std::string> Models;

	ModelsList();

	ModelsDirectory BaseDir;

};
