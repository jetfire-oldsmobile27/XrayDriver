// main.cpp
#include "xraytubecontroller.h"
#include "commandservice.h"
#include "daemonizer.h"
#include <boost/asio.h>

int main(int argc, char* argv[]) {
    try {
        if (argc > 1 && std::string(argv[1]) == "--daemon") {
            Daemonizer::daemonize();
        }

        boost::asio::io_context io;
        
        // Инициализация контроллера
        XRayTubeController::instance().init(io, "/dev/ttyXR0");
        
        // Запуск сервисов
        CommandService cmd_service;
        HttpServer::instance().start(io, 8080);
        
        // Обработка сигналов
        boost::asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io.stop(); });
        
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}