#pragma once
#include "xrayprotocolstrategy.h"
#include "iprotocolstrategy.h"
#include "service/db.h"
#include "eventnotifier.h"
#include "singleton.h"  
#include <boost/asio/serial_port.hpp>
#include <chrono>

class XRayTubeController : public Singleton<XRayTubeController> {
public:
    void init(boost::asio::io_context& io);
    void emergency_stop();
    bool is_exposure_active() const;
    void restart_driver();
    bool test_connection();
    void set_voltage(uint16_t kv);
    void set_current(float ma);
    void start_exposure(uint32_t duration_ms);
    XRayProtocolStrategy::Status get_status() const;
    std::chrono::milliseconds last_ping_time() const;

private:
    mutable std::mutex mutex_;
    std::unique_ptr<XRayProtocolStrategy> protocol_;
    std::chrono::milliseconds last_ping_time_{0};
};