#include "service/filesystem_utils.h"

namespace jetfire27::Engine::FilesystemUtils {

std::string GetAppData() {
  std::string path;
#ifdef _WIN32
  char appdata[MAX_PATH];
  if (SUCCEEDED(
          SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, appdata))) {
    path = std::string(appdata) + "\\XRAY\\XRAYDynamicDriver\\";
  }
#elif defined(__APPLE__)
  char *home = getenv("HOME");
  if (home) {
    path = std::string(home) + "/Library/Logs/XRAY/XRAYDynamicDriver/";
  }
#else
  char *home = getenv("HOME");
  if (home) {
    path = std::string(home) + "/.local/share/XRAY/XRAYDynamicDriver/";
  }
#endif
  return path;
}

std::string GetLogDirectory() {
#ifdef __APPLE__
  // macOS already returns log path
  return GetAppData();
#else
  return GetAppData() + "logs/";
#endif
}
std::string GetDefaultImagesPath() {
  std::string path = GetAppData() + "images";

  CreateDirectoryIfNotExists(path);

  return path;
}

bool CreateDirectoryIfNotExists(const std::string &path) {
#ifdef _WIN32
  return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
  return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

std::string GetDBPath() {
  CreateDirectoryIfNotExists(GetAppData());
  return GetAppData() + "xray_driver.db";
}


std::string getExecutablePath()
{
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return std::string(path);
#elif __APPLE__ || __linux__
    char path[4096];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1)
    {
        path[len] = '\0';
        return std::string(path);
    }
    return {};
#endif
}

} // namespace jetfire27::Engine::FilesystemUtils