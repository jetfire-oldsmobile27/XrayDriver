#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>

#include <shlobj.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

namespace jetfire27::Engine::FilesystemUtils {

std::string GetAppData();

std::string GetLogDirectory();
std::string GetDefaultImagesPath();
std::string GetDBPath();

bool CreateDirectoryIfNotExists(const std::string &path);

std::string getExecutablePath();

} // namespace jetfire27::Engine::FilesystemUtils