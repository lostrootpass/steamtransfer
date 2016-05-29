#ifndef UTIL_PLATFORM_H_
#define UTIL_PLATFORM_H_

#include <string>

bool isSteamRunning();

void copyApp(unsigned int id, unsigned int dest);

std::wstring getSteamInstallDirectory();

std::wstring getTextFileContents(const std::wstring& path);

#endif //UTIL_PLATFORM_H_