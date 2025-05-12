// include/driver/xrayprotocolstrategy.h
#pragma once
#include "iprotocolstrategy.h"
#include <boost/asio.hpp>

class XRayProtocolStrategy : public IProtocolStrategy {
public:
    XRayProtocolStrategy(boost::asio::serial_port& port);
    
    void set_voltage(uint16_t voltage);
    void start_exposure(uint32_t duration);
    
    // Реализация интерфейса
    void process(const std::vector<uint8_t>& data) override;
    void initialize() override;
    void shutdown() override;
    void reset_connection() override;

private:
    boost::asio::serial_port& m_port;
};