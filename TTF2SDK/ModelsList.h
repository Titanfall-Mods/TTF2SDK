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

struct Spawnlist
{
	std::string Name;
	std::string Path;
	bool CanWriteTo;
	std::vector<std::string> Props;
};

class ModelsList
{

public:

	ModelsList( FileSystemManager & fsManager );

	void LoadSpawnlists();

	void ChangeSpawnlist( std::string target );

	void AppendToSpawnlist( Spawnlist & appendTarget, std::string newModel );

	std::vector<Spawnlist> Spawnlists;

	Spawnlist * CurrentSpawnlist;

	ModelsDirectory BaseDir;

protected:

	static std::string DefaultSpawnlist;

	void UpdateSpawnlist( Spawnlist & newSpawnlist );

	FileSystemManager * m_fsManager;

	std::unordered_map< std::string, std::string > SpawnlistsFileMap;

};
