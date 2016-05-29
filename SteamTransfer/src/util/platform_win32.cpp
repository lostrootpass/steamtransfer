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

//Copy a SteamApp from its current location to the destination specified.
//The destination must be the numeric ID of a dest listed in libraryfolders.vdf
void copyApp(unsigned int id, unsigned int dest)
{
	//1. Find the folder containing id

	std::wstring installDir = getSteamInstallDirectory();
	std::wstring libFolderPath = installDir + L"/steamapps/libraryfolders.vdf";

	std::wstring contents = getTextFileContents(libFolderPath);

	VDFParser parser;
	VDFObject root;
	parser.parse(contents, root);

	std::wstring& universe = root[L"LibraryFolders"][L"1"].rawValue;

	struct stat s;
	if (stat(std::string(universe.begin(), universe.end()).c_str(), &s) == 0)
	{

	}

	//Make sure the source and dest aren't the same universe.

	//2. Make sure the folder doesn't already exist in the desination

	//3. Copy the metadata file between the steamapps directories

	//use std::rename

	//4. Copy folder to dest

	//use std::rename

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

	return retVal;
}

std::wstring getTextFileContents(const std::wstring& path)
{
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