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
        proto.set_voltage(voltage);
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
