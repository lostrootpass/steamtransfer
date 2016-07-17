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
	std::wstring installLocation;
	std::vector<std::wstring> libraryFolders;
	std::vector<SteamApp> appCache;

	void generateCache();

	unsigned int getUniverseIdFromDriveLetter(wchar_t driveLetter);

private:

	void _generateLibraryFolderCache();
};

#endif //STEAM_INFO_H_