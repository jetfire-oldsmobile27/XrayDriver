// xrayprotocolstrategy.cpp
#include "driver/xrayprotocolstrategy.h"
#include "service/logger.h"
using namespace jetfire27::Engine::Logging;

XRayProtocolStrategy::XRayProtocolStrategy(boost::asio::io_context &io, const std::string &port)
    : port_(io), port_name_(port), current_status_{} {}

bool XRayProtocolStrategy::test_connection()
{
    try
    {
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            last_response_.clear();
        }
        send_command("PING");
        auto response = wait_response(500);
        return response == "PONG";
    }
    catch (...)
    {
        return false;
    }
}

void XRayProtocolStrategy::process(const std::vector<uint8_t> &data)
{
    // Пока пусто
}

void XRayProtocolStrategy::initialize()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (port_.is_open())
        port_.close();

    try
    {
        port_.open(port_name_);
        port_.set_option(boost::asio::serial_port::baud_rate(38400));
        port_.set_option(boost::asio::serial_port::character_size(8));
        port_.set_option(boost::asio::serial_port::parity(
            boost::asio::serial_port::parity::none));
        port_.set_option(boost::asio::serial_port::stop_bits(
            boost::asio::serial_port::stop_bits::one));

        boost::asio::async_read_until(port_, read_buffer_, '\n',
                                      [this](auto ec, auto size)
                                      { read_handler(ec, size); });

        Logger::GetInstance().Info("Port {} initialized", port_name_);
    }
    catch (const boost::system::system_error &e)
    {
        std::string msg = "Failed to open port '" + port_name_ + "': ";
        msg += e.what();

   #ifdef _WIN32
        msg += "\nCheck COM port format: use 'COMx' (e.g., COM3)";
   #endif
        current_status_.error_state = true;
        Logger::GetInstance().Error("Controller init failed: {}", msg);
        last_error_ = msg;
    }
}

void XRayProtocolStrategy::shutdown()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (port_.is_open())
    {
        port_.close();
    }
    current_status_ = Status{};
}

void XRayProtocolStrategy::reset_connection()
{
    shutdown();
    initialize();
}

void XRayProtocolStrategy::reset_fault() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    send_command("RESET_FAULT");
    auto response = wait_response(500);
    if (response.find("FAULT_RESET_OK") == std::string::npos) {
        throw std::runtime_error("Fault reset failed");
    }
    current_status_.error_state = false;
}

void XRayProtocolStrategy::set_voltage(uint16_t kv)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!port_.is_open()) {
        Logger::GetInstance().Error("Port not open!");
        return;
    }
    send_command(fmt::format("SET_VOLTAGE {}", kv));
    auto response = wait_response(500);
    if (response.find("VOLTAGE_SET:") == std::string::npos) {
        throw std::runtime_error("Set voltage confirmation failed");
    }
    current_status_.voltage_kv = kv;
}

void XRayProtocolStrategy::set_current(float ma) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (ma < 0.01f || ma > 0.4f)
        throw std::runtime_error("Invalid current");
    send_command(fmt::format("SET_CURRENT {:.2f}", ma));
    auto response = wait_response(500);
    if (response.find("SET_CURRENT:") == std::string::npos) {
        throw std::runtime_error("Set voltage confirmation failed");
    }
    current_status_.current_ma = ma;
}

void XRayProtocolStrategy::start_exposure(uint32_t duration_ms)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!port_.is_open()) {
        Logger::GetInstance().Error("Port not open!");
        return;
    }
    if (current_status_.voltage_kv < 10)
        throw std::runtime_error("Voltage not set");

    send_command(fmt::format("START_EXPOSURE {}", duration_ms));
    auto response = wait_response(500);
    if (response.find("EXPOSURE_STARTED:") == std::string::npos) {
        throw std::runtime_error("Start exposure confirmation failed");
    }
    current_status_.exposure_active = true;
}

void XRayProtocolStrategy::emergency_stop()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    send_command("EMERGENCY_STOP");
    auto response = wait_response(500);
    if (response.find("EMERGENCY_STOP") == std::string::npos) {
        throw std::runtime_error("Start exposure confirmation failed");
    }
    current_status_.exposure_active = false;
}

void XRayProtocolStrategy::send_command(const std::string &cmd)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!port_.is_open()) return;

    try {
        last_response_.clear();
        boost::asio::write(port_, boost::asio::buffer(cmd + "\r\n"));
        auto response = wait_response(500);
    if (response.find("ACK") == std::string::npos) {
        throw std::runtime_error("No confirmation received");
    }
    } catch (const std::exception& e) {
        last_error_ = e.what();
        Logger::GetInstance().Error("Send failed: {}", e.what());
    }
    Logger::GetInstance().Info("Sent command: {}", cmd);
}

void XRayProtocolStrategy::read_handler(const boost::system::error_code &ec, size_t bytes)
{
    if (!ec)
    {
        std::istream is(&read_buffer_);
        std::string response;
        std::getline(is, response);

        if (!response.empty() && response.back() == '\r') {
            response.pop_back();
        }
        parse_response(response);

        boost::asio::async_read_until(port_, read_buffer_, '\n',
                                      [this](auto ec, auto size)
                                      { read_handler(ec, size); });
    }
    else
    {
        Logger::GetInstance().Error("Port read error: {}", ec.message());
    }
}

void XRayProtocolStrategy::parse_response(const std::string &response)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Logger::GetInstance().Info("Parse response from hardware xray controller: {}", response);
    last_response_ = response;
    response_condition_.notify_all();  // работает с std::condition_variable_any

    if (response.find("OK") != std::string::npos)
    {
        Logger::GetInstance().Debug("Received OK");
    }
    else if (response.find("ERROR") != std::string::npos)
    {
        current_status_.error_state = true;
        Logger::GetInstance().Error("Controller error: {}", response);
    }
    else if (response.find("OVERCURRENT") != std::string::npos)
    {
        emergency_stop();
        Logger::GetInstance().Critical("Overcurrent detected!");
    }
}

std::string XRayProtocolStrategy::wait_response(int timeout_ms)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    if ( response_condition_.wait_for(
           lock,
           std::chrono::milliseconds(timeout_ms),
           [this] { return !last_response_.empty(); }
         ) )
    {
        return std::move(last_response_);
    }
    return "";
}

IProtocolStrategy::Status XRayProtocolStrategy::get_status() const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return {
        current_status_.voltage_kv,
        current_status_.current_ma,
        current_status_.exposure_active,
        current_status_.filament_on,
        current_status_.error_state
    };
}

std::string XRayProtocolStrategy::last_error() const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return last_error_;
}
