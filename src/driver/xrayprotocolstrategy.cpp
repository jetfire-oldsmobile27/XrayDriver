#include "driver/xrayprotocolstrategy.h"
#include <boost/asio/serial_port.hpp>


XRayProtocolStrategy::XRayProtocolStrategy(boost::asio::serial_port& port,
                                            const std::string& port_name
) 
    : m_port(port), m_port_name(port_name) {}

void XRayProtocolStrategy::initialize() {
    m_port.set_option(boost::asio::serial_port::baud_rate(38400));
    m_port.set_option(boost::asio::serial_port::character_size(8));
    m_port.set_option(boost::asio::serial_port::parity(
        boost::asio::serial_port::parity::none));
    m_port.set_option(boost::asio::serial_port::stop_bits(
        boost::asio::serial_port::stop_bits::one));
    
    if (!m_port.is_open()) {
        throw std::runtime_error("COM port not opened!");
    }
}

void XRayProtocolStrategy::set_power(float power) {
    // Код для отправки команды регулировки мощности
}

void XRayProtocolStrategy::process(const std::vector<uint8_t>& data) {
    // Реализация обработки входящих данных
    // Например, отправка на порт:
    boost::asio::write(m_port, boost::asio::buffer(data));
}

void XRayProtocolStrategy::shutdown() {
    if(m_port.is_open()) {
        m_port.close();
    }
}

void XRayProtocolStrategy::reset_connection() {
    boost::system::error_code ec;

    if (m_port.is_open())
        m_port.close(ec);

    m_port.open(m_port_name, ec);
    if (ec) {
        throw boost::system::system_error(ec, "Failed to reopen port " + m_port_name);
    }

    initialize();
}