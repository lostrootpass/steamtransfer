#include "platform.h"

#if _WIN32

#include <cassert>

#include <windows.h>
#include <WinUser.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <fstream>
#include <codecvt>
#include <sys/stat.h>
#include <iostream>
#include <atlbase.h>
#include <ShlObj.h>
#include <algorithm>
#include "parse.h"

bool isSteamRunning()
{
	// http://stackoverflow.com/questions/865152/how-can-i-get-a-process-handle-by-its-name-in-c

	// Create toolhelp snapshot.
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process;
	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);

	bool found = false;

	// Walkthrough all processes.
	if(Process32First(snapshot, &process))
	{
		do
		{
			//Pitfall: will return true if *any* process called Steam.exe is running
			//But how many users run multiple things called Steam.exe on their boxes?
			//You could query Steam's process ID from the registry instead, if you really wanted.
			//Not sure how reliably that gets updated though (e.g. crash exits). This seems more trustworthy overall.
			if(strcmp(process.szExeFile, "Steam.exe") == 0)
			{
				found = true;
				break;
			}
		} while(Process32Next(snapshot, &process));
	}

	CloseHandle(snapshot);
	return found;
}

std::wstring folderNameFromManifest(const std::wstring& manifest)
{
	std::wstring contents = getTextFileContents(manifest);

	VDFParser parser;
	VDFObject root;
	parser.parse(contents, root);

	return root[L"AppState"][L"installdir"].rawValue;
}

//Copy a SteamApp from its current location to the destination specified.
//The destination must be the numeric ID of a dest listed in libraryfolders.vdf
void copyApp(unsigned int id, unsigned int dest)
{
	//Fun fact, we use backslashes here because some of the shell APIs will fail with E_INVALIDARG on
	// forward slashes even though many of the shell APIs understand forward slashes perfectly.
	//Funner fact: the above fun fact is entirely undocumented, anywhere.
	//Thanks, Microsoft.
	//Thicrosoft.

	std::wstring installDir = getSteamInstallDirectory();
	std::wstring libFolderPath = installDir + L"\\steamapps\\libraryfolders.vdf";
	std::wstring metadataFileName = L"appmanifest_" + std::to_wstring(id) + L".acf";

	std::wstring contents = getTextFileContents(libFolderPath);

	VDFParser parser;
	VDFObject root;
	parser.parse(contents, root);

	unsigned int currUniverse = -1;
	std::wstring sourcePath;
	std::wstring metaPath;
	struct stat s;

	//TODO: accurately retrieve universe list from VDF
	for (unsigned int u = 0; u <= 1; ++u)
	{
		//Construct the basic path of the universe's steamapps directory.
		std::wstring path = (u == 0 ? installDir : root[L"LibraryFolders"][std::to_wstring(u).c_str()].rawValue);
		path += L"\\steamapps\\";

		//Check if the app we're looking for is installed here.
		metaPath = path + metadataFileName;
		if (stat(std::string(metaPath.begin(), metaPath.end()).c_str(), &s) == 0)
		{
			//We've found it.
			currUniverse = u;
			sourcePath = path;
			break;
		}
	}

	//TODO: should probably notify the user somehow if these happen.
	if (currUniverse == -1)
	{
		std::cerr << "Couldn't find game install anywhere." << std::endl;
		return;
	}
	else if (currUniverse == dest)
	{
		std::cerr << "Game is already installed at given destination." << std::endl;
		return;
	}

	//Copy the metadata file to the destination first. It makes it easier to recover if the game copy goes wrong midway.
	//The appmanifest file contains its own Universe property, but we intentionally avoid changing that
	// Steam is smart enough to pick up that the universe for the game has changed and will update it itself.
	//Also, copying a tiny file like this across volumes is a good canary to avoid spending lots of time trying to
	// do a copy operation that might not even be allowed.
	
	std::wstring destPath = (dest == 0 ? installDir : root[L"LibraryFolders"][std::to_wstring(dest).c_str()].rawValue);
	destPath += L"\\steamapps\\";

	//Grab the path from the manifest before we move it.
	const std::wstring commonPath = L"common\\";
	std::wstring gamePath = commonPath + folderNameFromManifest((sourcePath + metadataFileName)) + L"\\";

	BOOL success = MoveFileWithProgressW((sourcePath + metadataFileName).c_str(), (destPath + metadataFileName).c_str(),
		NULL, NULL, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);

	char buf[260];
	if (!success)
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);

		std::cerr << "Failed to copy app manifest. Error code: " << GetLastError() << " " << buf <<std::endl;
		return;
	}

	//Now copy the game to the destination - we need the very verbose and tedious shell functions here
	//because MoveFileWithProgressW doesn't allow copying directories across volumes.
	//We also need progress indicators because we're potentially transferring tens of gigabytes and std::rename
	//doesn't provide that. Finally, we need the security descriptors to be transferred alongside the directories.


	//Make it fast. Gaming machines have RAM, let's suck it up.
	CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY);

	IFileOperation* fileOp;
	HRESULT hres = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&fileOp));
	if (FAILED(hres))
	{
		//...
	}

	fileOp->SetOperationFlags(FOFX_MOVEACLSACROSSVOLUMES | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION);

	CComPtr<IShellItem> sourceShellItem, destShellItem;

	hres = SHCreateItemFromParsingName((sourcePath + gamePath).c_str(), NULL, IID_PPV_ARGS(&sourceShellItem));
	if (FAILED(hres))
	{
		//...
	}

	hres = SHCreateItemFromParsingName((destPath + commonPath).c_str(), 0, IID_PPV_ARGS(&destShellItem));
	if (FAILED(hres))
	{
		//...
	}

	hres = fileOp->MoveItem(sourceShellItem, destShellItem, NULL, NULL);
	if (FAILED(hres))
	{
		//...
	}

	hres = fileOp->PerformOperations();
	if (FAILED(hres))
	{
		//if the manifest move succeeded but the game move didn't, should we attempt to revert?
		//I think no for now, because Steam can recover itself and redownload the 'missing' bits of the game.
		return;
	}

	std::cout << "Game copy completed successfully." << std::endl;
}

std::wstring getSteamInstallDirectory()
{
	std::wstring retVal = L"";

	HKEY key;
	LONG res = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_READ, &key);

	if(SUCCEEDED(res))
	{ 
		WCHAR buff[MAX_PATH];
		DWORD buffSize = MAX_PATH;

		DWORD ret = RegQueryValueExW(key, L"SteamPath", NULL, 0, (LPBYTE)buff, &buffSize);

		if(SUCCEEDED(ret))
			retVal = std::wstring(buff);
	}
	else //failed
	{
		if(res == ERROR_FILE_NOT_FOUND)
		{
			//Steam isn't installed? That sounds wrong. This should be handled somehow.
			//Probably best to just ask the user where their Steam install is.
		}
	}

	RegCloseKey(key);

	//Mandatory for some shell APIs that complain too much for their own good.
	std::replace(retVal.begin(), retVal.end(), '/', '\\');

	return retVal;
}

std::wstring getTextFileContents(const std::wstring& path)
{
	//TODO: lots of widestrings everywhere used unnecessarily. Bit messy. Could get rid of them.

	std::wstring contents = L"";

	std::wifstream stream(path);
	
	if(!stream.is_open())
		return contents;

	//All Valve-generated VDF files are UTF8.
	stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>()));

	std::wstring line;
	while(std::getline(stream, line))
	{
		contents += std::wstring(line.begin(), line.end());
	}

	return contents;
}

#endif