#ifndef UTIL_PLATFORM_H_
#define UTIL_PLATFORM_H_

#include <string>
#include <vector>

bool isSteamRunning();

void copyApp(unsigned int id, unsigned int dest);

std::wstring getSteamInstallDirectory();

std::wstring getTextFileContents(const std::wstring& path);

bool getAllManifests(const std::wstring& path, std::vector<std::wstring>& vector);

#endif //UTIL_PLATFORM_H_