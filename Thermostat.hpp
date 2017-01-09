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
#pragma mark - Thermostat Methods
    // ================================================================
    virtual Mode mode() const;
    void setMode(const Mode mode = Off);
    
    // This method returns the temperature as measured by this instance.
    // This means the value is dependent on the measurement type.
    virtual Temperature<int> temperature(int const sensorID = -1) const;
    
    virtual Temperature<int> targetTemperature() const;
    virtual void setTargetTemperature(Temperature<int> const targetTemperature);
    
    Measurement measurementType() const;
    void setMeasurementType(Measurement const measurementType = TemperatureUnit);
    
    //Sensor sensor(unsigned short i = 0);
    
    Thermostat(Scheduler::Time const executeTimeInterval = 5);
    virtual ~Thermostat();
    
protected:
    Mode _mode;
    Temperature<int> _targetTemperature;
    Measurement _measurmentType;
    Scheduler _scheduler;
    
    std::vector<int> _sensor;
//    TemperatureSensor *_sensor;
    
    // ================================================================
#pragma mark - Scheduler::Event Methods
    // ================================================================
    int execute(Scheduler::Time const updateTime,
                Scheduler::Time const updateDelay);
};

#endif /* Thermostat_hpp */
