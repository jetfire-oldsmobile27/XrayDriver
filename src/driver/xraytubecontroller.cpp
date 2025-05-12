#include "../../include/driver/xraytubecontroller.h"
#include "driver/xraytubecontroller.h"
#include "driver/xrayprotocolstrategy.h"

void XRayTubeController::init(boost::asio::io_context& io, const std::string& port) {
    m_port = std::make_unique<boost::asio::serial_port>(io, port);
    m_protocol = std::make_unique<XRayProtocolStrategy>(*m_port);
    // Дополнительная инициализация
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