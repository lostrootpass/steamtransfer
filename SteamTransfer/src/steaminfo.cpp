#include "steaminfo.h"
#include "util/platform.h"
#include "util/parse.h"
#include <sstream>
#include <locale>

void SteamInfo::generateCache()
{
	installLocation = getSteamInstallDirectory();
	_generateLibraryFolderCache();
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

unsigned int SteamInfo::getUniverseIdFromDriveLetter(wchar_t driveLetter)
{
	std::locale loc;
	unsigned int id = 0;
	for (std::wstring& it : libraryFolders)
	{
		if (std::tolower(it[0], loc) == std::tolower(driveLetter, loc))
			return id;

		++id;
	}

	return -1;
}