// XRayProtocolStrategy.hpp
#pragma once
#include "iprotocolstrategy.hpp"
#include <queue>
#include <boost/asio.hpp>

class XRayProtocolStrategy : public IProtocolStrategy {
public:
    XRayProtocolStrategy(boost::asio::serial_port& port) 
        : m_port(port) {}

    void process(const std::vector<uint8_t>& data) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_command_queue.push(data);
        process_next_command();
    }

    void set_voltage(uint16_t voltage) {
        std::string cmd = "VOLT:" + std::to_string(voltage) + "\n";
        send_command(cmd);
    }

    void start_exposure(uint32_t duration_ms) {
        std::string cmd = "EXPOSE:" + std::to_string(duration_ms) + "\n";
        send_command(cmd);
    }

private:
    void send_command(const std::string& cmd) {
        boost::asio::write(m_port, boost::asio::buffer(cmd));
        start_read_ack();
    }

    void start_read_ack() {
        boost::asio::async_read_until(m_port, m_buffer, '\n',
            [this](boost::system::error_code ec, size_t bytes) {
                if (!ec) {
                    std::istream is(&m_buffer);
                    std::string ack;
                    std::getline(is, ack);
                    handle_ack(ack);
                }
            });
    }

    void handle_ack(const std::string& ack) {
        if (ack != "OK") {
            EventNotifier::instance().notify({"COMMAND_FAILED", ack});
        }
    }

    boost::asio::serial_port& m_port;
    std::mutex m_mutex;
    std::queue<std::vector<uint8_t>> m_command_queue;
    boost::asio::streambuf m_buffer;
};