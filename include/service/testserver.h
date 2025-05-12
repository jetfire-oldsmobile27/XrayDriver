#pragma once
#include "driver/xraytubecontroller.h"
#include "service/logger.h"
#include "service/db.h"
#include <boost/asio.hpp>

namespace jetfire27::Engine::Test {
    class TestServer {
    public:
        void SetupHardwareInterface(boost::asio::io_context& io) {
            // Инициализация оборудования
            XRayTubeController::instance().init(io, "/dev/ttyXR0");
            
            // Регистрация API endpoints
            AddCommandHandlers();
        }

    private:
        void AddCommandHandlers() {
            // Обработчик команд для оборудования
            AddRoute("/api/command", 
                [this](const auto& req, auto& res) {
                    handleCommand(req, res);
                });
        }

        void handleCommand(const HttpRequest& req, HttpResponse& res) {
            try {
                auto cmd = parseCommand(req.body());
                XRayTubeController::instance().execute_command(cmd);
                sendSuccess(res);
            } catch (const std::exception& e) {
                sendError(res, e.what());
            }
        }
    };
}