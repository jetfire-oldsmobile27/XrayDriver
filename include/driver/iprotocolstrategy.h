#pragma once
#include <vector>
#include <cstdint>

class IProtocolStrategy {
public:
    virtual ~IProtocolStrategy() = default;
    
    virtual void process(const std::vector<uint8_t>& data) = 0;
    
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    
    virtual void reset_connection() = 0;
};