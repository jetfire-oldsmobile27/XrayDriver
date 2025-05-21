#pragma once
#include "xrayprotocolstrategy.h"
#include "iprotocolstrategy.h"
#include "eventnotifier.h"
#include "singleton.h"  
#include <boost/asio/serial_port.hpp>

class Event;  

class XRayTubeController : public Singleton<XRayTubeController> {
public:
    void init(boost::asio::io_context& io, const std::string& port);
    void execute_command(const std::string& cmd);

private:
    void handle_hardware_event(const Event& e);
    
    std::unique_ptr<boost::asio::serial_port> m_port;
    std::string m_choosen_port{"COM3"};
    std::unique_ptr<IProtocolStrategy> m_protocol;
};