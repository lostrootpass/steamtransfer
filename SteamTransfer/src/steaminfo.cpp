#include "steaminfo.h"
#include "util/platform.h"
#include "util/parse.h"
#include <sstream>
#include <locale>

void SteamInfo::generateCache()
{
	installLocation = getSteamInstallDirectory();
	_generateLibraryFolderCache();
	_generateAppCache();
}

unsigned int SteamInfo::getUniverseIdFromDriveLetter(wchar_t driveLetter) const
{
	std::locale loc;
	unsigned int id = 0;
	for (std::wstring it : libraryFolders)
	{
		if (std::tolower(it[0], loc) == std::tolower(driveLetter, loc))
			return id;

		++id;
	}

	return -1;
}

unsigned int SteamInfo::getAppIdFromName(const std::wstring& name) const
{
	//Look up from our cache what the app ID is.
	//It will either be a display name or a folder name.
	//TODO: Display names won't necessarily match up directly, as users won't type (tm) etc.
	// some kind of fuzzy search would be useful

	for (const SteamApp app : appCache)
	{
		if (app.displayName == name || app.folderName == name)
		{
			return app.id;
		}
	}

	return -1;
}

const SteamApp* SteamInfo::getApp(const std::wstring& name) const
{
	for (const SteamApp& app : appCache)
	{
		if (app.displayName == name || app.folderName == name)
		{
			return &app;
		}
	}

	return 0;
}

void SteamInfo::_generateLibraryFolderCache()
{
	std::wstring installDir = getSteamInstallDirectory();
	std::wstring libFolderPath = installDir + L"\\steamapps\\libraryfolders.vdf";

	std::wstring contents = getTextFileContents(libFolderPath);

	VDFParser parser;
	VDFObject root;
	parser.parse(contents, root);

	libraryFolders.insert(libraryFolders.begin(), installLocation);

	const std::unordered_map<std::wstring, VDFObject>& map = root[L"LibraryFolders"].getAll();
	unsigned int id;
	for (auto it : map)
	{
		std::wstringstream ss(it.first);
		ss >> id;
		if (!ss.fail())
		{
			libraryFolders.insert(libraryFolders.begin() + id, it.second);
		}
	}
}

void SteamInfo::_generateAppCache()
{
	VDFParser parser;

	for (const std::wstring& folder : libraryFolders)
	{
		std::vector<std::wstring> manifests;
		if (getAllManifests(folder, manifests))
		{
			for (const std::wstring& manifest : manifests)
			{
				std::wstring contents = getTextFileContents(folder + L"\\steamapps\\" + manifest);
				if (contents != L"")
				{
					VDFObject root;
					parser.parse(contents, root);
					
					SteamApp app;
					VDFObject appstate = root[L"AppState"];
					std::wstringstream ss(appstate[L"appid"]);
					ss >> app.id;
					app.currentUniverse = folder;
					app.displayName = appstate[L"name"];
					app.folderName = appstate[L"installdir"];

					appCache.push_back(app);
				}
			}
		}
	}
}
