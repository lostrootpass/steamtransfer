//Copyright(c) 2016 Pete Murdoch http://pmurdoch.com/
//
//zlib license:
//
//This software is provided 'as-is', without any express or implied
//warranty. In no event will the authors be held liable for any damages
//arising from the use of this software.
//
//Permission is granted to anyone to use this software for any purpose,
//including commercial applications, and to alter it and redistribute it
//freely, subject to the following restrictions:
//
//1. The origin of this software must not be misrepresented; you must not
//   claim that you wrote the original software. If you use this software
//   in a product, an acknowledgement in the product documentation would be
//   appreciated but is not required.
//2. Altered source versions must be plainly marked as such, and must not be
//   misrepresented as being the original software.
//3. This notice may not be removed or altered from any source distribution.

#include <iostream>
#include <clocale>
#include <QtWidgets/QApplication>

#include "ui/MainWindow.h"
#include "util/platform.h"
#include "steaminfo.h"

int cmdLineExec(int argc, char** argv);

int main(int argc, char** argv)
{
	std::setlocale(LC_ALL, "en_US.UTF-8");

#if 0 //Disable the UI until it works at least a little bit.
	if(argc < 2)
	{
		//Launched directly, so show the UI.
		QApplication app(argc, argv);

		MainWindow window;
		window.init();
		window.show();

		return app.exec();
	}
	else
#endif
	{
		//Run from the command line.
		//QCommandLineParser is unreliable and EXTREMELY buggy,	hence DIY
		
		return cmdLineExec(argc, argv);
	}

	return 0;
}

int cmdLineExec(int argc, char** argv)
{
	unsigned int appid = -1;
	unsigned int dest = -1;

	std::wstring name = L"";
	std::wstring destName = L"";

	for(int i = 0; i < argc; i++)
	{
		char* arg = argv[i];

		//If called with arg names, make sure there's an arg after the name
		if(arg && i + 1 <= argc && argv[i + 1] && *argv[i + 1])
		{
			if(strcmp(arg, "--appid") == 0)
			{
				appid = atoi(argv[i + 1]);
			}
			else if(strcmp(arg, "--dest") == 0)
			{
				dest = atoi(argv[i + 1]);

				if(dest == 0)
				{
					std::string str(argv[i + 1]);
					destName = std::wstring(str.begin(), str.end());
				}
			}
			else if(strcmp(arg, "--name") == 0)
			{
				std::string str(argv[i + 1]);
				name = std::wstring(str.begin(), str.end());
			}
		}
	}

	SteamInfo info;
	info.generateCache();

	//Convert provided name to actual app ID
	if(appid == -1 && name != L"")
	{
		appid = info.getAppIdFromName(name);

		if (appid == -1)
			std::cerr << "Wasn't able to find game install based on name. Try using the app ID instead." << std::endl;
	}

	//Converted provided dest to one indexed in libraryfolders.vdf
	if(destName != L"")
	{
		dest = info.getUniverseIdFromDriveLetter(destName[0]);

		if (dest == -1)
			std::cerr << "Didn't detect Steam library on specified drive. Are you sure it exists?" << std::endl;
	}

	if(appid != -1 && dest != -1)
	{
		if(!isSteamRunning())
			copyApp(appid, dest);
		else
		{
			std::cerr << "Error: cannot copy data while Steam is running." << std::endl;
		}
	}

	std::cin.get();
	return 0;
}