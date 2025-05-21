#pragma once
#include "iprotocolstrategy.h"
#include "xrayprotocolstrategy.h"

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(IProtocolStrategy& proto) = 0;
};

class SetVoltageCommand : public Command {
public:
    SetVoltageCommand(uint16_t v) : voltage(v) {}
    
    void execute(IProtocolStrategy& proto) override { 
        if (auto xray_proto = dynamic_cast<XRayProtocolStrategy*>(&proto)) {
            xray_proto->set_voltage(voltage);
        } else {
            throw std::runtime_error("Invalid protocol type!");
        }
    }

private:
    uint16_t voltage;
};

class ExposureCommand : public Command {
public:
    ExposureCommand(uint32_t d) : duration(d) {}
    
    void execute(IProtocolStrategy& proto) override {
        dynamic_cast<XRayProtocolStrategy&>(proto).start_exposure(duration);
    }

private:
    uint32_t duration;
};

class SetPowerCommand : public Command {
public:
    SetPowerCommand(float p) : power(p) {}
    
    void execute(IProtocolStrategy& proto) override {
        dynamic_cast<XRayProtocolStrategy&>(proto).set_power(power);
    }

private:
    float power;
};