#include "service/testserver.h"
#include "service/daemonizer.h"
#include "service/logger.h"
#include <boost/asio.hpp>

std::string getExecutablePath() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return std::string(path);
#elif __APPLE__ || __linux__
    char path[4096];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)-1);
    if (len != -1) {
        path[len] = '\0';
        return std::string(path);
    }
    return {};
#endif
}


int main() {
    using namespace jetfire27::Engine;
    
    Logging::Logger::GetInstance().Initialize("/var/log/jetservice");
    
    if (Daemonizer::IsSingleInstance()) {
        Daemonizer::Setup(getExecutablePath(), Mode::Service);
    }

    boost::asio::io_context io;
    
    Test::TestServer server(8080, "/var/db/test.db");
    server.SetupHardwareInterface(io);
    server.Run();
    
    io.run();
}