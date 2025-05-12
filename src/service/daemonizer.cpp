#include "daemonizer.h"

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#else
    #include <fstream>
    #include <unistd.h>
    #include <sys/file.h>
    #include <sys/stat.h>
#endif

using namespace jetfire27::Engine;

namespace {
    constexpr const char* SERVICE_NAME = "jetservice";
    constexpr const char* DESKTOP_FILE = "jetservice.desktop";
#ifdef _WIN32
    HANDLE instanceMutex = nullptr;
#else
    int lockFd = -1;
#endif
}

void Daemonizer::Setup(const std::string& binPath, Mode mode) {
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Configuring daemon mode: {}", static_cast<int>(mode));
#ifdef _WIN32
    if (mode == Mode::AutoStart) {
        HKEY hKey;
        RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_WRITE, &hKey);
        RegSetValueExA(hKey, SERVICE_NAME, 0, REG_SZ,
                       (const BYTE*)binPath.c_str(), (DWORD)(binPath.size() + 1));
        RegCloseKey(hKey);
    } else {
        SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return;

    SC_HANDLE hService = CreateServiceA(
        hSCManager,
        SERVICE_NAME,
        "JetService Windows Daemon",
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        binPath.c_str(),
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (!hService) {
        hService = OpenServiceA(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
        if (hService) {
            ChangeServiceConfigA(hService,
                SERVICE_WIN32_OWN_PROCESS,
                SERVICE_AUTO_START,
                SERVICE_ERROR_NORMAL,
                binPath.c_str(),
                nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        }
    }

    if (hService) {
        StartServiceA(hService, 0, nullptr);
        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCManager);
    }
#elif __APPLE__
    std::string plistPath = std::string(getenv("HOME")) + "/Library/LaunchAgents/" + SERVICE_NAME + ".plist";
    std::ofstream f(plistPath);
    f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
         "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         "<plist version=\"1.0\">\n"
         "<dict>\n"
         "    <key>Label</key>\n"
         "    <string>" << SERVICE_NAME << "</string>\n"
         "    <key>ProgramArguments</key>\n"
         "    <array><string>" << binPath << "</string></array>\n"
         "    <key>RunAtLoad</key>\n"
         "    <true/>\n"
         "</dict>\n"
         "</plist>\n";
#else
    if (mode == Mode::AutoStart) {
        std::string path = std::string(getenv("HOME")) + "/.config/autostart/" + DESKTOP_FILE;
        std::ofstream f(path);
        f << "[Desktop Entry]\n"
             "Type=Application\n"
             "Exec=" << binPath << "\n"
             "X-GNOME-Autostart-enabled=true\n"
             "Hidden=false\n";
    } else {
        std::ofstream f("/etc/systemd/system/" + std::string(SERVICE_NAME) + ".service");
        f << "[Unit]\nDescription=JetService Daemon\n\n"
             "[Service]\nExecStart=" << binPath << "\nRestart=always\n\n"
             "[Install]\nWantedBy=multi-user.target\n";
        // Activate via systemctl if needed
        // system("systemctl daemon-reexec && systemctl enable --now jetservice-driver");
    }
#endif
}

void Daemonizer::Remove(Mode mode) {
#ifdef _WIN32
    if (mode == Mode::AutoStart) {
        HKEY hKey;
        RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0, KEY_WRITE, &hKey);
        RegDeleteValueA(hKey, SERVICE_NAME);
        RegCloseKey(hKey);
    } else {
        SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCManager) return;

    SC_HANDLE hService = OpenServiceA(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);
    if (hService) {
        SERVICE_STATUS status;
        ControlService(hService, SERVICE_CONTROL_STOP, &status);
        DeleteService(hService);
        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCManager);
    }
#elif __APPLE__
    std::string plistPath = std::string(getenv("HOME")) + "/Library/LaunchAgents/" + SERVICE_NAME + ".plist";
    std::remove(plistPath.c_str());
#else
    if (mode == Mode::AutoStart) {
        std::string path = std::string(getenv("HOME")) + "/.config/autostart/" + DESKTOP_FILE;
        std::remove(path.c_str());
    } else {
        std::string svcPath = "/etc/systemd/system/" + std::string(SERVICE_NAME) + ".service";
        std::remove(svcPath.c_str());
        // Optionally disable service
        // system("systemctl disable --now jetservice-driver");
    }
#endif
}

bool Daemonizer::IsSingleInstance() {
#ifdef _WIN32
    instanceMutex = CreateMutexA(nullptr, TRUE, "Global\\jetserviceMutex");
    return (GetLastError() != ERROR_ALREADY_EXISTS);
#else
    std::string lockFile = "/tmp/" + std::string(SERVICE_NAME) + ".lock";
    lockFd = open(lockFile.c_str(), O_CREAT | O_RDWR, 0666);
    if (lockFd < 0) return false;
    if (flock(lockFd, LOCK_EX | LOCK_NB) != 0) return false;
    return true;
#endif
}
