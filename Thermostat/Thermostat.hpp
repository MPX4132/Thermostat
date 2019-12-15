//
//  Thermostat.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#ifndef Thermostat_hpp
#define Thermostat_hpp

#include <utility>
#include <vector>
#include "Development.hpp"
#include "Temperature.hpp"
#include "Thermometer.hpp"
#include "Scheduler.hpp"
#include "Identifiable.hpp"
#include "Actuator.hpp"

#if defined(MJB_ARDUINO_LIB_API)
#include <Arduino.h>
#else
#include <iostream>
#endif

// =============================================================================
// Thermostat : This class abstracts the functionality of an HVAC control system
// and provides a simplistic interface for interacting with it.
// =============================================================================
class Thermostat : protected Scheduler::Daemon
{
public:
    // ================================================================
    // Thermostat Types
    // ================================================================
    typedef std::vector<std::shared_ptr<Thermometer>> Thermometers;
    typedef std::pair<Thermometer::TemperatureUnit::value_type,
                      Thermometer::TemperatureUnit::Scale> TemperatureThreshold;
    
    enum Mode
    {
        Off,
        Heat,
        Cool,
        Auto
    };
    
    enum Status
    {
        Standby,
        Heating,
        Cooling,
        Stasis
    };

    enum PerceptionIndex
    {
        TemperatureIndex,   // Control based on temperature
        HeatIndex           // Control based on heat index
    };

    enum SignalLine
    {
        FanCall,
        CoolCall,
        HeatCall
    };

    enum ExecutionCode
    {
        Success,
        SignalLinesMissing,     // Not enough pins (3; cool, fan, heat)
        SignalLinesNotReady,    // A pin isn't ready to actuate.
    };
    
    // ================================================================
    // Thermostat Members
    // ================================================================
    Thermometers thermometers;
    
    
    // ================================================================
    // Thermostat Methods
    // ================================================================
    Mode mode() const;
    void setMode(Mode const mode = Off);
    
    Status status() const;
    
    // This method returns the average humidity of all thermometers.
    Thermometer::TemperatureUnit::value_type humidity();
    
    // This method returns the average humiture of all thermometers.
    Thermometer::TemperatureUnit humiture();
    
    // This method returns the average temperature of all thermometers.
    Thermometer::TemperatureUnit temperature();
    
    Thermometer::TemperatureUnit targetTemperature() const;
    void setTargetTemperature(Thermometer::TemperatureUnit const &targetTemperature);

    TemperatureThreshold targetTemperatureThreshold() const;
    void setTargetTemperatureThreshold(TemperatureThreshold const &targetTemperatureThreshold);

    PerceptionIndex perceptionIndex() const;
    void setPerceptionIndex(PerceptionIndex const perceptionIndex = TemperatureIndex);
    
    int update(Scheduler::Time const time);
    
    // The pin order is as follows by default: {FAN call, COOL call, HEAT call}
    // By default, the thermostat updates every 5 minutes (300000000us).
    Thermostat(Pin::Arrangement const &pins,
               Thermometers const &thermometers,
               Scheduler::Time const executeTimeInterval = 300000000);
    virtual ~Thermostat();
    
protected:
    Thermometer::TemperatureUnit _targetTemperature;
    TemperatureThreshold _targetTemperatureThreshold;
    PerceptionIndex _perceptionIndex;
    Status _status;
    Mode _mode;

    Actuator _controller;

    Scheduler _scheduler;
    
    Status _standby(Status const status = Standby);
    Status _setCooler(bool const cool);
    Status _setHeater(bool const heat);
    
    // ================================================================
    // Scheduler::Event Methods
    // ================================================================
    int execute(Scheduler::Time const updateTime);
};

#endif /* Thermostat_hpp */

