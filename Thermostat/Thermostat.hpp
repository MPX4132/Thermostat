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
    
    enum Measurement
    {
        TemperatureUnit,	// Control based on temperature
        HeatIndexUnit		// Control based on heat index
    };
    
    
    // ================================================================
    // Thermostat Members
    // ================================================================
    Thermometers thermometers;
    
    
    // ================================================================
    // Thermostat Methods
    // ================================================================
    virtual Mode mode() const;
    void setMode(const Mode mode = Off);
    
    Status status() const;
    
    // This method returns the average temperature of the thermometers.
    virtual Temperature<float> temperature();
    
    virtual Temperature<float> targetTemperature() const;
    virtual void setTargetTemperature(Temperature<float> const targetTemperature,
                                      // About a degree (F/C) of threshold
                                      float const targetTemperatureThreshold = 1);
    
    Measurement measurementType() const;
    void setMeasurementType(Measurement const measurementType = TemperatureUnit);
    
    // The pin order is as follows by default: {FAN call, COOL call, HEAT call}

    Thermostat(Pin::Arrangement const &pins,
               Thermometers const &thermometers,
#ifdef HARDWARE_INDEPENDENT
               Scheduler::Time const executeTimeInterval = 5);
#else
               Scheduler::Time const executeTimeInterval = 5000000);
#endif
    virtual ~Thermostat();
    
protected:
    Temperature<float> _targetTemperature;
    float _targetTemperatureThreshold;
    Measurement _measurmentType;
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
    
    void _reflectUpdates();
    
    // ================================================================
    // Scheduler::Event Methods
    // ================================================================
    int execute(Scheduler::Time const updateTime);
};

#endif /* Thermostat_hpp */

