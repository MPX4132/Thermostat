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
#include "Thermometer.hpp"
#include "Temperature.hpp"
#include "Scheduler.hpp"

// =============================================================================
// Thermostat : This class abstracts the functionality of an HVAC control system
// and provides a simplistic interface for interacting with it.
// =============================================================================
class Thermostat : protected Scheduler::Daemon
{
public:
    // ================================================================
#pragma mark - Thermostat Types
    // ================================================================
    typedef std::vector<Thermometer> Thermometers;
    
    enum Mode
    {
        Off,
        Cooling,
        Heating,
        Auto
    };
    
    enum Measurement
    {
        TemperatureUnit,	// Control based on temperature
        HeatIndexUnit		// Control based on heat index
    };
    
    
    // ================================================================
#pragma mark - Thermostat Members
    // ================================================================
    Thermometers thermometers;
    
    
    // ================================================================
#pragma mark - Thermostat Methods
    // ================================================================
    virtual Mode mode() const;
    void setMode(const Mode mode = Off);
    
    // This method returns the average temperature of the thermometers.
    virtual Temperature<float> temperature();
    
    virtual Temperature<float> targetTemperature() const;
    virtual void setTargetTemperature(Temperature<float> const targetTemperature);
    
    Measurement measurementType() const;
    void setMeasurementType(Measurement const measurementType = TemperatureUnit);
    
    Thermostat(Thermometers &thermometers, Scheduler::Time const executeTimeInterval = 5);
    virtual ~Thermostat();
    
protected:
    Mode _mode;
    Temperature<float> _targetTemperature;
    Measurement _measurmentType;
    Scheduler _scheduler;
    
    
    // ================================================================
#pragma mark - Scheduler::Event Methods
    // ================================================================
    int execute(Scheduler::Time const updateTime,
                Scheduler::Time const updateDelay);
};

#endif /* Thermostat_hpp */
