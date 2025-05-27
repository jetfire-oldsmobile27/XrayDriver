#pragma once
#include <vector>
#include <cstdint>

class IProtocolStrategy {
public:
    virtual ~IProtocolStrategy() = default;
    
    virtual void set_voltage(uint16_t kv) = 0;
    virtual void set_current(float ma) = 0;
    virtual void start_exposure(uint32_t duration_ms) = 0;
    virtual void emergency_stop() = 0;
    
    virtual void process(const std::vector<uint8_t>& data) = 0;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual void reset_connection() = 0;
    
    virtual bool is_exposure_active() const = 0;
    virtual struct Status {
        uint16_t voltage_kv;
        float current_ma;
        bool exposure_active;
        bool error_state;
    } get_status() const = 0;
};