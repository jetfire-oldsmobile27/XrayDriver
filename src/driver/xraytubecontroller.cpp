#include "driver/xraytubecontroller.h"
#include "service/logger.h"
#include "driver/xrayprotocolstrategy.h"

void XRayTubeController::init(boost::asio::io_context& io, const std::string& port) {
    try {
        m_port = std::make_unique<boost::asio::serial_port>(io, port);
        m_port->open(m_choosen_port);
        m_protocol = std::make_unique<XRayProtocolStrategy>(*m_port, m_choosen_port);
        m_protocol->initialize();
    } catch (const std::exception& e) {
        jetfire27::Engine::Logging::Logger::GetInstance().Error(
            "Port {} initialization failed: {}", port, e.what());
        throw;
    }
}

void XRayTubeController::execute_command(const std::string& cmd) {
    // Базовая реализация
    if (cmd == "start") {
        m_protocol->initialize();
    }
}

void XRayTubeController::handle_hardware_event(const Event& e) {
    // Обработка событий
}