#include "../include/service/testserver.h"
#include "../include/service/daemonizer.h"

int main() {
    Logger::Initialize("/var/log/jetservice");
    
    if (Daemonizer::IsSingleInstance()) {
        Daemonizer::SetupAsService();
    }

    boost::asio::io_context io;
    
    TestServer server;
    server.SetupHardwareInterface(io);
    server.Start(8080);
    
    io.run();
}