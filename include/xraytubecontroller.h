// XRayTubeController.hpp
#pragma once
#include "protocolstrategy.hpp"
#include "eventnotifier.hpp"
#include <memory>

class XRayTubeController : public Singleton<XRayTubeController> {
public:
    void init(boost::asio::io_context& io, const std::string& port) {
        m_port = std::make_unique<boost::asio::serial_port>(io, port);
        m_protocol = std::make_unique<XRayProtocolStrategy>(*m_port);
        
        m_port->set_option(boost::asio::serial_port::baud_rate(38400));
        EventNotifier::instance().subscribe(
            [this](const auto& e) { handle_hardware_event(e); });
    }

    void execute_command(const std::string& cmd) {
        static CommandFactory factory;
        auto command = factory.create(cmd);
        if (command) {
            command->execute(*m_protocol);
        }
    }

private:
    void handle_hardware_event(const Event& e) {
        if (e.type == "EMERGENCY_STOP") {
            m_protocol->send_command("STOP\n");
        }
    }

    std::unique_ptr<boost::asio::serial_port> m_port;
    std::unique_ptr<IProtocolStrategy> m_protocol;
};