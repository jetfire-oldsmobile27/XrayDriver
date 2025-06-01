#include "driver/xraytubecontroller.h"
#include "service/logger.h"
#include "service/db.h"
#include "driver/xrayprotocolstrategy.h"

void XRayTubeController::init(boost::asio::io_context &io,
                              std::shared_ptr<jetfire27::Engine::DB::SQLiteDB> db)
{
    auto &log = jetfire27::Engine::Logging::Logger::GetInstance();
    log.Info("Initializing X-Ray controller");
    try
    {
        std::string port;

        db->Execute("SELECT value FROM xray_settings WHERE key='com_port'", [](void *data, int, char **vals, char **)
                    {
                if(vals[0]) *static_cast<std::string*>(data) = vals[0];
                return 0; }, &port);

        if (port.empty())
        {
#ifdef _WIN32
            port = "COM1"; // Значение по умолчанию для Windows
#else
            port = "/dev/ttyUSB0"; // Значение по умолчанию для Linux
#endif
            log.Info("Using default COM port: {}", port);
        }

        // Проверка корректности имени порта для Windows
#ifdef _WIN32
        if (port.find("COM") != 0 || port.size() < 4)
        {
            throw std::runtime_error("Invalid COM port format. Use COMx format");
        }
#endif

        protocol_ = std::make_unique<XRayProtocolStrategy>(io, port);
        protocol_->initialize();
    }
    catch (const std::exception &e)
    {
        log.Error("X-Ray controller failed: {}", e.what());
        IProtocolStrategy::Status hardware_status = protocol_->get_status();
        if (hardware_status.error_state) {
            log.Error("Hardware connection failed with message: {}", protocol_->last_error());
        };
        throw;
    }
    catch (...)
    {
        throw std::runtime_error("X-Ray controller unhandled throw");
    }
    log.Info("X-Ray controller initialized success");
}

void XRayTubeController::emergency_stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    protocol_->emergency_stop();
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Emergency stop confirmed");
}

bool XRayTubeController::is_exposure_active() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return protocol_->is_exposure_active();
}

void XRayTubeController::restart_driver()
{
    using namespace std::chrono_literals;

    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        if (protocol_) {
            protocol_->shutdown();
            protocol_->initialize(); 
        }
        jetfire27::Engine::Logging::Logger::GetInstance().Info("Hardware connection restarted");
    } catch (const std::exception& e) {
        last_error_ = e.what();
        jetfire27::Engine::Logging::Logger::GetInstance().Error("Hardware restart failed: {}", e.what());
    }

}

void XRayTubeController::resetFault() {
    std::lock_guard<std::mutex> lock(mutex_);
    protocol_->reset_fault();
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Reset emergency confirmed");
}

bool XRayTubeController::test_connection()
{
    try
    {
        auto start = std::chrono::steady_clock::now();
        protocol_->send_command("PING");
        auto response = protocol_->wait_response(1000);
        last_ping_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);

        return response.find("PONG") != std::string::npos;
    }
    catch (const std::exception &e)
    {

        jetfire27::Engine::Logging::Logger::GetInstance().Error("Connection test failed: {}", e.what());
        return false;
    }
}

bool XRayTubeController::is_connected() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return protocol_ && protocol_->is_connected();
}

std::string XRayTubeController::last_error() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return last_error_;
}

void XRayTubeController::set_voltage(uint16_t kv)
{
    std::lock_guard<std::mutex> lock(mutex_);
    protocol_->set_voltage(kv);
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Voltage set to {} kV confirmed", kv);
}

void XRayTubeController::set_current(float ma)
{
    std::lock_guard<std::mutex> lock(mutex_);
    protocol_->set_current(ma);
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Current set to {} ma confirmed", ma);
}

void XRayTubeController::start_exposure(uint32_t duration_ms)
{
    std::lock_guard<std::mutex> lock(mutex_);
    protocol_->start_exposure(duration_ms);
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Exposure with duration {} ms confirmed", duration_ms);
}

IProtocolStrategy::Status XRayTubeController::get_status() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return protocol_->get_status(); // Возвращаем статус из протокола
}

std::chrono::milliseconds XRayTubeController::last_ping_time() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return last_ping_time_;
}
