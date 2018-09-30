//
//  Thermostat.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#ifndef Thermostat_hpp
#define Thermostat_hpp

#include <vector>
#include "Development.hpp"
#include "Temperature.hpp"
#include "Thermometer.hpp"
#include "Scheduler.hpp"
#include "Identifiable.hpp"
#include "Actuator.hpp"

#ifdef HARDWARE_INDEPENDENT
#include <iostream>
#else
#include <Arduino.h>
#endif

// =============================================================================
// Thermostat : This class abstracts the functionality of an HVAC control system
// and provides a simplistic interface for interacting with it.
// =============================================================================
class Thermostat : protected Actuator, protected Scheduler::Daemon
{
public:
    // ================================================================
    // Thermostat Types
    // ================================================================
    typedef std::vector<Thermometer *> Thermometers;
    
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
    float humidity();
    
    // This method returns the average humiture of all thermometers.
    Thermometer::TemperatureUnit humiture();
    
    // This method returns the average temperature of all thermometers.
    Thermometer::TemperatureUnit temperature();
    
    Thermometer::TemperatureUnit targetTemperature() const;
    void setTargetTemperature(Thermometer::TemperatureUnit const targetTemperature,
                              // About a degree (F/C) of threshold
                              float const targetTemperatureThreshold = 1);

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
    float _targetTemperatureThreshold;
    PerceptionIndex _perceptionIndex;
    Status _status;
    Mode _mode;
    
    // This scheduler shadows the Actuator scheduler, which is what we
    // need since the Actuator auto-deletes all events that are done.
    // That means when the Thermostat finishes it would crash because
    // the Actuator would try to delete the auto-generated instance.
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

