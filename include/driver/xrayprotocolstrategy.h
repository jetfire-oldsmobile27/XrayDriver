// xrayprotocolstrategy.h
#pragma once
#include "iprotocolstrategy.h"
#include <boost/asio.hpp>
#include <atomic>
#include <mutex>

class XRayProtocolStrategy : public IProtocolStrategy {
public:
    XRayProtocolStrategy(boost::asio::io_context& io, const std::string& port);
    
    bool test_connection();
    // Основные команды
    void set_voltage(uint16_t kv);
    void set_current(float ma);
    void start_exposure(uint32_t duration_ms);
    void emergency_stop();
    void set_filament(bool state);
    
    // Реализация интерфейса
    void process(const std::vector<uint8_t>& data) override;
    void initialize() override;
    void shutdown() override;
    void reset_connection() override;

    std::string wait_response(int timeout_ms);
    bool is_exposure_active() const { return current_status_.exposure_active; }

    // Состояние системы
    struct Status {
        uint16_t voltage_kv;
        float current_ma;
        bool exposure_active;
        bool filament_on;
        bool error_state;
    };

    IProtocolStrategy::Status get_status() const override;
    
    void restart_driver();
    std::string last_error() const;
    void send_command(const std::string& cmd);
    void read_handler(const boost::system::error_code& ec, size_t bytes);
    void parse_response(const std::string& response);
    bool is_connected() const { return port_.is_open(); }

private:
    

    mutable std::mutex mutex_;
    boost::asio::serial_port port_;
    Status current_status_;
    std::string port_name_;
    boost::asio::streambuf read_buffer_;
    std::string last_response_;
    std::string last_error_;
    std::condition_variable response_condition_;
};