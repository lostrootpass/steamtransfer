#ifndef STEAM_INFO_H_
#define STEAM_INFO_H_

#include <vector>
#include <string>

struct SteamApp
{
	unsigned int id;
	std::wstring displayName;
	std::wstring folderName;
	std::wstring currentUniverse;
};

struct SteamInfo
{
	std::wstring installLocation;
	std::vector<std::wstring> libraryFolders;
	std::vector<SteamApp> appCache;

	void generateCache();

	unsigned int getUniverseIdFromDriveLetter(wchar_t driveLetter) const;

	unsigned int getAppIdFromName(const std::wstring& name) const;

private:

	void _generateLibraryFolderCache();

	void _generateAppCache();
};

#endif //STEAM_INFO_H_