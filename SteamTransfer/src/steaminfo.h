#ifndef STEAM_INFO_H_
#define STEAM_INFO_H_

#include <vector>
#include <string>

struct SteamApp
{
	unsigned int id;
	std::string name;
	std::string currentLibraryFolder;
};

struct SteamInfo
{
	std::string installLocation;
	std::vector<std::string> libraryFolders;
	std::vector<SteamApp> appCache;
};

#endif //STEAM_INFO_H_