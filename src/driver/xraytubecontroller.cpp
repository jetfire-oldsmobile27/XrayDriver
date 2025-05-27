#include "driver/xraytubecontroller.h"
#include "service/logger.h"
#include "driver/xrayprotocolstrategy.h"

void XRayTubeController::init(boost::asio::io_context& io) {
    DB::SQLiteDB db("settings.db");
    std::string port;
    
    db.Execute("SELECT value FROM xray_settings WHERE key='com_port'",
        [](void* data, int, char** vals, char**) {
            if(vals[0]) *static_cast<std::string*>(data) = vals[0];
            return 0;
        }, &port);

    protocol_ = std::make_unique<XRayProtocolStrategy>(io, port);
    protocol_->initialize();
}

void XRayTubeController::emergency_stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    protocol_->emergency_stop();
}

bool XRayTubeController::is_exposure_active() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return protocol_->is_exposure_active();
}

void XRayProtocolStrategy::set_current(float ma) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(ma < 0.01f || ma > 0.4f) throw std::runtime_error("Invalid current");
    send_command(fmt::format("SET_CURRENT {:.2f}", ma));
    current_status_.current_ma = ma;
}

void XRayTubeController::restart_driver() {
    using namespace std::chrono_literals;

    std::lock_guard<std::mutex> lock(mutex_);
    if(is_exposure_active()) {
        emergency_stop();
        std::this_thread::sleep_for(500ms);
    }

    if(protocol_) {
        protocol_->shutdown();
        protocol_->initialize();
    }
    
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Driver restarted");
}

bool XRayTubeController::test_connection() {
    try {
        auto start = std::chrono::steady_clock::now();
        protocol_->send_command("PING");
        auto response = protocol_->wait_response(1000);
        last_ping_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        
        return response.find("PONG") != std::string::npos;
    } catch(const std::exception& e) {
        
        jetfire27::Engine::Logging::Logger::GetInstance().Error("Connection test failed: {}", e.what());
        return false;
    }
}

