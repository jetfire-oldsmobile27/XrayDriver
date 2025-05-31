#include "service/testserver.h"
#include "service/daemonizer.h"
#include "service/logger.h"
#include <memory>
#include <boost/asio.hpp>
#include <csignal>

#define _SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS

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

std::function<void()> shutdown_handler;
void signal_handler(int) { shutdown_handler(); }

int main()
{
    using namespace jetfire27::Engine;

    Logging::Logger::GetInstance().Initialize("C:\\Users\\dimaw\\Downloads\\logger");

    try {
        if (Daemonizer::IsSingleInstance())
        {
            Daemonizer::Setup(getExecutablePath(), Mode::Service);
        }

        boost::asio::io_context io;
        boost::asio::signal_set signals(io, SIGINT, SIGTERM);
        std::shared_ptr<Test::TestServer> server = std::make_shared<Test::TestServer>(8080, "C:\\Users\\dimaw\\Downloads\\logger\\base.db");
        shutdown_handler = [&]
        {
            io.stop();
            server->Stop();
        };

        signals.async_wait([&](auto, auto)
                           { shutdown_handler(); });
        std::thread server_thread([&]
                                  { 
        server->SetupHardwareInterface(io);
        server->Run(); });
        io.run();
        server_thread.join();
    }catch (const std::exception& e) {
        Logging::Logger::GetInstance().Critical("XRAY Driver initialization failed: {}", e.what());
        return 1;
    }
    while(true) {}; // event loop
    return 0;
}