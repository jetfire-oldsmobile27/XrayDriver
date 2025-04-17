// CommandService.hpp
#pragma once
#include "httpserver.hpp"
#include "xraytubecontroller.hpp"

class CommandService {
public:
    CommandService() {
        HttpServer::instance().add_route(
            "/api/command",
            [this](const auto& req, auto& res) { handle_command(req, res); });
    }

    void handle_command(const HttpRequest& req, HttpResponse& res) {
        try {
            auto cmd = nlohmann::json::parse(req.body());
            XRayTubeController::instance().execute_command(cmd["type"], cmd["params"]);
            res.body = {{"status", "OK"}};
        } catch (const std::exception& e) {
            res.result = http::status::bad_request;
            res.body = {{"error", e.what()}};
        }
    }
};