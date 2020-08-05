
#include "stdafx.h"
#include "ModelsList.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

std::string ModelsList::DefaultSpawnlist = "all-models";

ModelsList::ModelsList( FileSystemManager & fsManager )
{
	m_fsManager = &fsManager;

	LoadSpawnlists();
	ChangeSpawnlist( ModelsList::DefaultSpawnlist );
}

void ModelsList::LoadSpawnlists()
{
	Spawnlists.clear();

	for( auto & dirIter : fs::recursive_directory_iterator( m_fsManager->GetSpawnlistsPath() ) )
	{
		if( dirIter.status().type() == fs::file_type::directory )
		{
			continue;
		}

		// Get the path of the spawnlist file
		fs::path path = dirIter.path();
		std::string pathString = path.string();
		std::string fileName = Util::Split( pathString, '\\' ).back();
		fileName = Util::Split( fileName, '.' ).front();

		// Load the spawnlist contents
		std::ifstream spawnlistFileStream( pathString );
		std::string spawnlistData = Util::ReadFileToString( spawnlistFileStream );

		// Parse the spawnlist and add to the props list
		std::vector<std::string> parsedLines = Util::Split( spawnlistData, '\n' );
		std::vector<std::string> propsList;
		for( auto & model : parsedLines )
		{
			if( model.length() > 0 )
			{
				propsList.push_back( model );
			}
		}

		// Save the spawnlist and add the spawnlist to arrays for use in ui
		Spawnlist newSpawnlist;
		newSpawnlist.Name = fileName;
		newSpawnlist.Path = pathString;
		newSpawnlist.Props = propsList;
		newSpawnlist.CanWriteTo = fileName != ModelsList::DefaultSpawnlist;
		Spawnlists.push_back( newSpawnlist );
	}
}

void ModelsList::ChangeSpawnlist( std::string target )
{
	for( Spawnlist & list : Spawnlists )
	{
		if( list.Name == target )
		{
			UpdateSpawnlist( list );
			break;
		}
	}
}

void ModelsList::AppendToSpawnlist( Spawnlist & appendTarget, std::string newModel )
{
	spdlog::get( "logger" )->info( "Appending model {} to spawnlist {}", newModel, appendTarget.Name );

	if( std::find( appendTarget.Props.begin(), appendTarget.Props.end(), newModel ) == appendTarget.Props.end() )
	{
		appendTarget.Props.push_back( newModel );
	}

	std::ofstream outfile;
	outfile.open( appendTarget.Path, std::ios_base::app );
	outfile << "\n" << newModel;
}

void ModelsList::UpdateSpawnlist( Spawnlist & newSpawnlist )
{
	CurrentSpawnlist = &newSpawnlist;

	BaseDir = ModelsDirectory();
	BaseDir.Path = "models/";

	// Parse models
	for( std::string & model : newSpawnlist.Props )
	{
		std::stringstream stream( model );
		std::string item;
		std::vector<std::string> modelPaths;
		while( std::getline( stream, item, '/' ) )
		{
			modelPaths.push_back( item );
		}

		ModelsDirectory * RelativePath = &BaseDir;
		for( int i = 1; i < modelPaths.size() - 1; ++i )
		{
			if( RelativePath->SubDirectories.count( modelPaths[i] ) == 0 )
			{
				ModelsDirectory newDir = ModelsDirectory();
				newDir.FullPath = RelativePath->FullPath + modelPaths[i] + "/";
				newDir.Path = modelPaths[i] + "/";
				RelativePath->SubDirectories.emplace( modelPaths[i], newDir );
			}
			RelativePath = &( RelativePath->SubDirectories[modelPaths[i]] );
		}

		RelativePath->Models.push_back( model );
		RelativePath->ModelNames.push_back( modelPaths[modelPaths.size() - 1] );
	}
}
